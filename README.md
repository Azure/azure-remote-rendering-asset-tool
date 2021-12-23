---
title: Azure Remote Rendering Toolkit
description: Azure Remote Rendering Toolkit welcome page
author: jakras
ms.author: jakras
ms.date: 12/23/2021
ms.topic: article
---

# Azure Remote Rendering Toolkit (ARRT)

![ARRT material editing view](Documentation/media/ARRT.png)

Azure Remote Rendering Toolkit (ARRT) is a desktop application developed in C++/Qt that demonstrates how to use [Azure Remote Rendering](https://docs.microsoft.com/azure/remote-rendering) (ARR). It can be used to:

* Upload files to Azure Storage
* Convert models for Azure Remote Rendering
* Create remote rendering sessions
* Preview remotely rendered 3D models
* Modify its materials
* See basic performance statistics

ARRT is meant as a sample application for how to integrate Azure Remote Rendering into C++ applications. However, regardless of how you intend to use remote rendering, ARRT can always be used to get basic file upload, conversion and preview tasks done.

## General Prerequisites

To use ARRT, you need a working remote rendering account. [Create an ARR account](https://docs.microsoft.com/azure/remote-rendering/how-tos/create-an-account) if you don't have one yet.

You should be familiar with the following ARR concepts:

* [Sessions](https://docs.microsoft.com/azure/remote-rendering/concepts/sessions)
* [Models](https://docs.microsoft.com/azure/remote-rendering/concepts/models)
* [Model conversion](https://docs.microsoft.com/azure/remote-rendering/how-tos/conversion/model-conversion)

## Prebuilt Binaries

Prebuilt ARRT binaries [can be found here](https://github.com/Azure/azure-remote-rendering-asset-tool/releases).

> **Important:**
>
> If running ARRT fails due to a missing DLL (`VCRUNTIME140_* dll`), please install the latest Visual C++ redistributable from [the Visual Studio download page](https://visualstudio.microsoft.com/downloads/) or using [this direct link](https://aka.ms/vs/16/release/VC_redist.x64.exe)

## Building ARRT

### Prerequisites

* [Visual studio 2019 or 2022](https://visualstudio.microsoft.com/downloads).
* [Qt 5.13.1 or newer](https://www.qt.io/download-qt-installer). Use the default installation options.
  * Set the `Qt5_DIR` environment variable (e.g. to `C:\5.13.1\msvc2017_64`).
* [CMake](https://cmake.org/download).
  * Make sure *cmake.exe* is in the `PATH` environment variable.
* [Command-line NuGet](https://www.nuget.org/downloads).
  * Make sure *nuget.exe* is in the `PATH` environment variable.
* HEVC driver. See the [ARR system requirements](https://docs.microsoft.com/azure/remote-rendering/overview/system-requirements) for details.

### Building

1. Clone this repository
1. Open a command line terminal
1. From the repository's root directory, run the script *GenerateSolution.bat \<OutputDirectory\> -vs2019*
    * The script will run CMake and generate a Visual Studio solution in *OutputDirectory*
    * Use *-vs2022* if you want to generate the solution for Visual Studio 2022
1. Open and compile the generated solution

## Documentation

* [ARRT User Documentation](Documentation/index.md)

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
