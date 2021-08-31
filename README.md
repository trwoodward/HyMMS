# HyMMS
An experimental hybrid generational/ incremental garbage collector aimed at game development - created for the final project of my MSc.

## Introduction

This is my final project from my MSc in Computer Science at Birkbeck College, University of London, from 2012. The code is offered for use under the MIT license without warranty. 

The project itself was something of a thought experiment, born out of playing with Microsoft's XNA game framework, which ran on C#/.Net and allowed indie developers to deploy and test games on their xbox 360s. 

One of the biggest 'gotcha's of the framework was trying to control when the Garbage Collector (GC) ran, because the GC on the xbox 360 in particular was not very sophisticated and could cause very choppy framerates. At the time (with the likes of the Unity game engine being relatively new on the scene) commercial game development was still dominated by C++ and manual memory management. Which got me thinking - what would a GC that was designed with game development in mind look like?

This project was the result of that thought experiment. The implementation is pretty naïve, especially since it was based on a not-fully-compliant version of C++11, and I'm sure GC design has moved on a lot since I undertook this project, but I'm making it available in case anyone else out there finds it interesting and/ or useful.

I have also included the source code for the demos that I used to test the performance of HyMMs, which are referenced in the project report. These almost certainly no longer compile using modern tools (I have not tested them recently), but are included for completeness.

## What is HyMMs?

In summary, HyMMs is a hybrid of a generational GC and an incremental GC - with regular, full sweeps made of a nursery generation, and any surviving objects then being handed off to an incremental compacting GC. The goal of the algorithm was to preserve space in memory (which is often at a premium in games) while spreading out the cost of the garbage collection as evenly as possible between frames, at a cost of potentially higher overall performance overheads.

For a full description of the algorithm used, see the full project report.
