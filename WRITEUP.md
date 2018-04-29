# Fuzzing Network Binaries

## Introduction

Within the vulnerability research field, fuzzing is an unparalleled tool to
discover new exploitable memory corruption bugs as well as other potentially
compromising issues in software. It is also a great tool to perform software
testing and to harden code against unforeseen conditions. In fact, Google
utilises the technique heavily with their OSS-Fuzz Project, [@google-ossfuzz] in
which continuous fuzzing of open source software is performed to ensure that
the components under test is free from low-hanging vulnerabilities.

Historically, fuzzing has been performed in a 'dumb' manner with the data
provided to the application under testing being generated in a random manner.
This is traditionally done in hopes that the program crashes, or halts
execution unexpectedly. At the very least this could indicate a
denial-of-service condition or it could be a sign of something more sinister,
such as memory corruption. In the context of a networked application, a
denial-of-service condition is extremely undesireable as it affects the
availability of the service rendered by the program. For instance,
CVE-2017-7687 [@amon-cve-2017-7687] has critical real life implications for the
various organisations that make use of Apache Mesos due to the trivial manner
an attacker can cause the engine to crash.

In recent times, however, advances in fuzzing has taken a smarter turn with
coverage-guided methods. The American Fuzzy Lop Fuzzer by the prolific Polish
vulnerability research Michał Zalewski [@afl-fuzz] is the pioneer of this
technique. It has since become one of the most popular and effective fuzzing
tools publicly available. Coverage-guided fuzzers work by instrumenting a
program binary (either at compile time or during execution with just-in-time
techniques) with beacons that keep track of how much of the program code is
executed during a run. The fuzzer attempts to mutate user input such that
coverage is maximised. This is the key to the successes of AFL and other
coverage-guided fuzzers. Since the technique lends itself to finding new paths
through the binary in a very systematic manner, it can turn up interesting test
cases that could potentially reveal a crash. Other coverage-based fuzzers
include libfuzzer [@llvm-libfuzzer], a project under the LLVM Compiler which
uses the instrumentation provided by LLVM's built in code coverage
capabilities.

Fuzzers featured heavily during the monumental 2016 Cyber Grand Challenge
organised by the U.S. Defense Advanced Research Projects Agency (DARPA) hosted
at DEFCON 24 in Las Vegas. Teams were required to create automated systems to
discover, exploit, and patch vulnerabilities in a custom restricted-scoped
implementation of the Linux operating system. These automated systems were not
allowed any human intervention during the competition. Thus, many of the teams
developed tools combining whitebox software testing techniques such as symbolic
and concolic execution, and coverage-based fuzzing. Driller [@driller] by the
University of California, Santa Barbara team is one of the examples of this. It
is towards this direction that the state-of-the-art fuzzers are moving towards.

## Challenges with Network Fuzzing

Fuzzing lends itself very well to operations that take in self-contained user
input such as image or file parsers and converters. Some of the applications
that are heavily fuzzed include PDF readers, audio and video codecs, image
conversion tools, and other such single-functionality programs. These programs
are typically not networked and do not rely on a communicated state.
Furthermore, these programs usually take in input through a file or through the
stdin, process the input, and then exit. This makes it easy for the fuzzer to
run through many iterations of the program.

When the network comes into the picture, things get a lot more complicated.
Certain factors like servers accepting multiple connections, applications
requiring state changes to happen on individual payloads sent or received in a
particular read or write, or even checks that happen in the program logic that
require certain conditions in the network to be satisfied like a valid
connected socket all contribute to the complexities of fuzzing a networked
binary.

## Current Solutions

There are a few solutions that attempt to deal with this complexity.

#### Non-Instrumented Fuzzing

This option involves using a 'dumb' or non-instrumented fuzzer to fuzz the
networked binary by simply connecting to or receiving a connection from the
target and sending mutated data from a seed input. This continues in a random
fashion until a crash is detected. The fuzzer is usually not protocol aware.
This is usually a very inefficient choice and it may take an unfeasibly long
time before crashes are observed with this method.

#### Protocol Specific Fuzzers

Some protocols are well-known enough that protocol specific fuzzers can be
created. Given a set of known inputs and structures, particular fields can be
fuzzed. This is useful for fuzzing different implementations of the same
protocol. However, these fuzzers are not broadly useful and may not catch bugs
that stem from bugs that are caused in inputs that diverge from the well
defined protocol structure.

#### Modified AFL

There exists a community modified version of AFL that allows for networked
fuzzing. [@afl-network] What it does is that it opens a socket connection to
the target and sends the mutated data in one write to the target, then closes
the connection. There are multiple issues with this. The most critical of the
issues is that programs that require a dialog will not be fuzzable under this
modified fuzzer.

#### Custom Harnesses

Another method involves ignoring the network step altogether and extracting the
critical parts of the program that the researcher believes is important. This
direct harness approach is arguably the best option when dealing with large and
complex binaries, even non-networked ones. This allows the fuzzer to side-step
irrelevant portions of the code and fuzz only the regions of the binary that
could potentially contribute greatly to the attack surface. This lends itself
very well to libraries. A disadvantage of this technique is that this requires a
lot of manual effort and analysis on the part of the researcher. The researcher
has to understand the binary and the code base to a considerable extent before
a harness can be written. Furthermore, in a networked binary, this could
possibly not be an option if the state is important to the logic of the binary.

#### Desocketing

A contribution by Yan Shoshitaishvili, Assistant Professor at the University of
California, Santa Barbara allows researchers to 'desocket' the binaries by
faking a socket connection and bridging the socket's file descriptors to the
stdin and stdout of the program. [@preeny] This allows for interaction with the
networked binary much like how a command line application would interact.
This is done with the LD\_PRELOAD environment variable in Linux and overrides
the standard socket standard functions like connect, bind, and accept. However,
this technique still stands at a disadvantage when dealing with binaries that
require stateful dialogs. In this paper, we utilise this desocketing approach
and modify it deal with these dialogs.

### Limitations

The approachs described above fall into two distinct categories of drawbacks:

1. They are too non-specific to be useful.
2. They do not take into account the dialog factor in a typical networked
   interaction with a binary.

## Applications of NDM to Fuzzing

When fuzzing networked applications that require multiple pieces of
conversation within a network dialog, eliminating redundant packets is key to
speeding up the fuzzing process. Each packet required to be replayed adds an
additional overhead as well as adds another possible data region to be fuzzed.
Only including the relevant packets improves the efficiency of our fuzzer. Once
a valid non-crashing network capture of a dialog between a client and the
target is obtained, the NDM process can be applied to the capture to produce a
subset. This minimised trace is then passed to the fuzzing step.

The minimised packet capture and the target binary is then used for fuzzing. In
our implementation, we use vanilla AFL. Before the fuzzing can occur, the
minimised packet capture has to be pre-processed to generate a meta index of
the dialog including the raw payload and direction of the transmission. This is
used during the fuzzing process.

To transform the target binary into a fuzzable target, we heavily modify the
desocketing shared library mentioned above.  First, the researcher selects one
or more dialog indexes to apply the fuzzing process to. During the process, all
packets except for the specified indexes are kept constant. Instead of bridging
the socket to the stdin and stdout file descriptors, we fake a network
connection in-process. Next, we allow the binary to run but keep track of an
index into the current network dialog in a seperate monitor thread. For each
transmit or receive in the dialog, we replay the transmission by 'mmaping' the
pre-processed data from a file into memory and then writing to the 'remote'
socket. However, instead of replaying the original data when reaching the
indices specified to be under test, we allow AFL to generate the test input for
that particular packet. This allows for AFL to employ its highly efficient
coverage-guided heurestics to generate the mutations that reach deep code
paths. This can be repeated for different selected indexes to fuzz.

The method described above now allows for the state before the payload under
test to be preserved. This vastly improves the fuzzing capability for networked
binaries without requiring comprehensive insight into nor modification of the
tested code base. All that is required is a minimised packet capture of a valid
session.

### Proof of Concept

A proof-of-concept can be found at this Github repository:
https://github.com/nnamon/afl-network-harness. It includes example vulnerable
binaries as well as the fuzzing outputs including the crashing test cases. It
utilises a heavily modified desocketing shared library found here and other
python helper scripts found here: https://github.com/nnamon/desock-replayer.
There are couple of limitations with the proof-of-concept including persistent
mode not being deployed in the preloaded shared library. This introduces a
slight de-optimisation in which the setup leading to the actual mutation has to
be repeated between each iteration of the program. Additionally, certain
sanitizers such as the Address Sanitizer do not work with the preload method in
its current form as they require initialisation before every other shared
library. These limitations are not insurmountable and will be addressed in a
future release.

