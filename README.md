---
title: Azure Remote Rendering asset tool (ARRT)
description: Azure Remote Rendering asset tool welcome page
author: mafranc
ms.author: mafranc
ms.date: 03/23/2020
ms.topic: article
---

# Azure Remote Rendering asset tool (ARRT)

![ARRT material editing view](Documentation/media/ARRT.png)

Azure Remote Rendering asset tool (ARRT) is a desktop application developed in C++/Qt that can be used to:

* control the model conversion
* create a remote rendering session
* load a model
* preview the model
* modify its materials

It can be used as a sample application to learn how to implement a front end for the ARR C++ SDK, using the [Azure Storage Client Library](https://github.com/Azure/azure-storage-cpp) for managing the 3D model conversion.

## General Prerequisites

To use ARRT, you need a working remote rendering account. See [Azure Remote Rendering account](https://docs.microsoft.com/en-us/azure/remote-rendering/how-tos/create-an-account) to create an ARR account.

You need to be familiar with the following key concepts:

* [Sessions](https://docs.microsoft.com/en-us/azure/remote-rendering/concepts/sessions)
* [Models](https://docs.microsoft.com/en-us/azure/remote-rendering/concepts/models)
* [Model conversion](https://docs.microsoft.com/en-us/azure/remote-rendering/how-tos/conversion/model-conversion)

## Prerequisites to compile and run ARRT from git repo

* Visual studio 2017 or newer
* Qt 5.13.1 or newer. Find the installation on the [website](https://www.qt.io/download-qt-installer). Use the default installation options. If you want to debug the Qt code, select the source code. Make sure the Qt5_DIR environment variable is set
* CMake version 3.16. Find the installation [here](https://cmake.org/download/). Make sure CMake is in the PATH
* Command-line NuGet from [here](https://www.nuget.org/downloads). Make sure Nuget.exe is in the PATH
* HEVC driver (if not present already). For more information about the system requirements for ARR, see [System requirements](https://docs.microsoft.com/en-us/azure/remote-rendering/overview/system-requirements)

## Compiling ARRT

* Clone the GitHub Repository [Azure Remote Rendering Asset Tool](https://github.com/Azure/azure-remote-rendering-asset-tool)
* From the root directory, run the script GenerateSolution.bat -vs2017. The script will run CMake and generate a Visual Studio solution you can open with Visual Studio 2017
* Compile ARRT from Visual Studio (Debug or Release)
* Run

## Documentation

* [ARRT User Documentation](Documentation/index.md)

## Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
