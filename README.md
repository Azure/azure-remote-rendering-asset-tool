---
title: Azure Remote Rendering Asset Tool (ARRT)
description: Azure Remote Rendering Asset Tool welcome page
author: mafranc
ms.author: mafranc
ms.date: 03/23/2020
ms.topic: article
---

# Azure Remote Rendering Asset Tool (ARRT)

![ARRT material editing view](Documentation/media/ARRT.png)

Azure Remote Rendering Asset Tool (ARRT) is a desktop application developed in C++/Qt that can be used to:

* Convert models for ARR
* Create a remote rendering session
* Preview a model
* Modify its materials

It can be used as a sample application to learn how to implement a frontend for the ARR C++ SDK, using the [Azure Storage Client Library](https://github.com/Azure/azure-storage-cpp) for managing the 3D model conversion.

## General Prerequisites

To use ARRT, you need a working remote rendering account. See [Azure Remote Rendering account](https://docs.microsoft.com/azure/remote-rendering/how-tos/create-an-account) to create an ARR account.

You need to be familiar with the following key concepts:

* [Sessions](https://docs.microsoft.com/azure/remote-rendering/concepts/sessions)
* [Models](https://docs.microsoft.com/azure/remote-rendering/concepts/models)
* [Model conversion](https://docs.microsoft.com/azure/remote-rendering/how-tos/conversion/model-conversion)

## Using a prebuilt version of ARRT

* Find the latest release in [the GitHub release page](https://github.com/Azure/azure-remote-rendering-asset-tool/releases).
* Download the `ARRT.zip` file from the release.
* Unzip to a directory of your choice.
* Run.

> **Important:**
>
> If running ARRT fails due to a missing DLL (`VCRUNTIME140_1.dll`), please install the latest Visual C++ redistributable from [the Visual Studio download page](https://visualstudio.microsoft.com/downloads/) or using [this direct link](https://aka.ms/vs/16/release/VC_redist.x64.exe)

## Building ARRT

### Prerequisites

* [Visual studio 2019 or 2022](https://visualstudio.microsoft.com/downloads).
* [Qt 5.13.1 or newer](https://www.qt.io/download-qt-installer). Use the default installation options. If you want to debug the Qt code, select the source code.
  * Set the `Qt5_DIR` environment variable (e.g. to `C:\5.13.1\msvc2017_64`).
* [CMake version 3.16 or newer](https://cmake.org/download/).
  * Make sure CMake is in the `PATH` environment variable.
* [Command-line NuGet](https://www.nuget.org/downloads).
  * Make sure Nuget.exe is in the `PATH` environment variable.
* HEVC driver. See the [ARR system requirements](https://docs.microsoft.com/azure/remote-rendering/overview/system-requirements) for details.

### Building

* Clone the GitHub Repository [Azure Remote Rendering Asset Tool](https://github.com/Azure/azure-remote-rendering-asset-tool).
* From the root directory, run the script *GenerateSolution.bat \<OutputDirectory\> -vs2019*. The script will run CMake and generate a Visual Studio solution in *OutputDirectory*.
* Open it with Visual Studio 2019.
* Compile (Debug or Release).
* Run.

## Documentation

* [ARRT User Documentation](Documentation/index.md)

## Other development utilities

If Clang and Ninja are installed locally, you have access a few useful dev tools:

* **clang-format**. It can be triggered on the whole codebase, running *FormatSourceCode.bat*.
* **Ninja build**. By running *BuildWithNinja.bat \<OutputDirectory\>*, ARRT will be automatically built in the output directory specified using Ninja.
* **clang-tidy**. You can call *RunClangTidy.bat* which will build with ninja and then run clang-tidy on all of the .cpp source files of ARRT.

You can download Clang from the [LLVM Download Page](https://releases.llvm.org/download.html). And Ninja-build from the [Ninja-build GitHub repo](https://github.com/ninja-build/ninja/releases).

## Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit <https://cla.opensource.microsoft.com>.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
