# InternalFlash

- [InternalFlash](#InternalFlash)
- [Location](#location)
- [Type](#type)
- [Background](#background)
- [Overview (all)](#overview-all)
- [Detailed Design (all)](#detailed-design-all)
  - [API (Interfaces/Implementation/Structure/Software Architecture)](#api-interfacesimplementationstructuresoftware-architecture)
  - [Platform Porting (platform)](#platform-porting-platform)
    - [Definitions File](#definitions-file)
    - [Linker Script](#linker-script)
    - [Host Communication: Stdout & stdin](#host-communication-stdout--stdin)
    - [Millisecond Counter](#millisecond-counter)
    - [Heap Allocation](#heap-allocation)
    - [FreeRTOS](#freertos)
    - [Interrupt Vector Table](#interrupt-vector-table)
    - [Writing .mk file](#writing-mk-file)
    - [Startup](#startup)
- [Caveats](#caveats)
- [Future Advancements (optional)](#future-advancements-optional)
- [Testing Plan](#testing-plan)
  - [Unit Testing Scheme (only for interface/system architecture)](#unit-testing-scheme-only-for-interfacesystem-architecture)
  - [Integration Testing (only for build)](#integration-testing-only-for-build)
  - [Demonstration Project (only for implementation/system architecture)](#demonstration-project-only-for-implementationsystem-architecture)

# Location
`L1 Peripheral`.

# Type
`Interface`

# Background
Flash memory is non-volatile memory, which retains the data even when power is cut off. The internal flash memory is located on the processor and will generally hold the bootloader. Other possible data like application code, variables, and configurations can also be stored here. (ISP and IAP here)

# Overview (all)
A short overview of:

1. What you are designing.
2. How it adds to code base.
3. Some examples of how this can be used, if applicable.

This document describes the `InternalFlash` abstraction. This will allow the user to modify the internal flash, providing more control over the sectors and load their own bootloader or update their application onto the microcontroller. 

An example of how `InternalFlash` can be used:
1. Update application code without having to flash the microcontroller through a cable.
    - If the application supports downloading an update, then the update can be applied to the application code using IAP.
2. Install a new bootloader

# Detailed Design (all)

## API (Interfaces/Implementation/Structure/Software Architecture)
Write the class/structure definition of the module. Do not include comments
here. Make this nice and simple. The names of methods and parameters should be
obvious to those experienced in the field.

```C++
class InternalFlash
{
 public:
  virtual void Initialize() = 0;
  virtual Status FlashSector(uint32_t start, uint32_t end) = 0;
  virtual Status FlashRead() = 0;
  virtual Status PrepareSector(uint32_t start, uint32_t end) = 0;
  virtual Status EraseSector(uint32_t start, uint32_t end) = 0;
  // ...
};
```

### void Initialize()
When called, this initializes the peripheral hardware. You must ensure that the
counter is set to zero after this call. NOTE: This must not enable the counter.
In order to start counting based on an external input, an explicit invocation of
`Enable()` must occur.

### Status FlashSector(uint32_t start, uint32_t end, uint32_t src)
Flashes sectors designated by 'start' and 'end'. Calls CopyRamToFlash (FlashBlock[4096 bytes]). Self calculates the intended size to flash into based on sector numbers.

### Status PrepareSector(uint32_t start, uint32_t end)
Prepares the sectors designated by 'start' and 'end for a `FlashSector` or `EraseSector` operation.

### Status EraseSEctor(uint32_t start, uint32_t end)
Erases sectors designated by 'start' and 'end'.

# Caveats
Explain any caveats about your design. This area is very important to make it
easy for those reading this design document to understand why they should or
should not use a particular design.
This area is explicitly here for providing known costs inherit in the design,
such as:

* Performance costs
    * Does this solution have any inheriant performance issues.
    * Example: Gpio Interrupt counter works well when used for something that is
      low frequency. If too many signals come in, or the pin is floating, then
      many interrupts willbe fired, leaving very little time for the processor
      to do work.
* Space costs
    * Does your module use up a lot of memory.
    * Example: SSD1308 OLED display driver makes a (128x64)/8 block of memory
* Compile time costs
    * Does this feature, when used, result in slower compile times.
    * Example: Use of this module utilizes complex template meta-programming and
      this results in an additional 2s of build time when compiling hello_world
      project.
* Usage costs
    * How difficult, complicated, or inelegant is using this module.
    * Example: Usage of this module requires coordination between 5 seperate
      modules, for which, if just 1 of them is not initialized correctly, will
      result in undefined behavior.
* Safety costs
    * Something about the feature that may invoke undefined behavior
    * Example: If the implementation's refresh method has not been called within
      5 seconds of each other, the behavior of the module will be undefined.
* Portability costs
    * Is this solution specific to a particular compiler
    * Example: The fixed sized sjsu::List<> object utilizes hidden std library
      structure definitions depending on the specific stl library or compiler,
      so using this on anything other than GCC or CLANG, it will fallback to
      another type, but that may not properly reflect the amount of memory being
      used.

# Future Advancements (optional)
Add a list of improvements that can improve or extend this further in the
future, state them here. Otherwise, put "N/A".

# Testing Plan
This section is irrelevant to **Interface** and **Structure** type documents.

## Unit Testing Scheme (only for interface/system architecture)
Please explain what techniques you plan on using to test the

## Integration Testing (only for build)
Write a plan for testing that the new build feature works across platforms.
Using TravisCI and checking manually that it completed as expected is the usual
approach.

## Demonstration Project (only for implementation/system architecture)
Write up simplified code example that demonstrates how your system will be used.
This should not be as detailed as your actual demonstration project. Should
be a small snippet that shows the simplest usage of your module.
