---
title: Uploading data with ARRT
description: Describes how to upload 3D models to Azure Storage with the Azure Remote Rendering Toolkit
author: jakras
ms.author: jakras
ms.date: 12/23/2021
ms.topic: article
---

# Upload models in Azure Remote Rendering asset tool

![Upload panel](media/upload.png)
To convert a 3D model, first it has to be uploaded to one the containers in your Azure Storage account. To do so, click on the "Upload" button on the main toolbar.

## Selection a container

Select the container where you want to upload the model using the combo-box on the top.
If you want to upload your files to a new container, you can press the "+" icon on the right side of the container combo-box. You can then type the name of the new container and press enter. If the container you typed exists, it will be selected, if it doesn't exist, it will be created as soon as you'll upload a file into it.

## Selection of the destination directory

To select a destination directory, you can navigate to it by double clicking on it in the blob list. Alternatively, you can click on the right arrow on one of the directory buttons, to navigate to one of its sub-directories.

To select a new directory, click on the "Add Sub-Directory" button, with a folder and a plus sign, enter the name of a new directory and press enter.

## Upload

To upload files from your local hard drive into the current root directory, you can:

* drag and drop the files directly into the list
* click on the "Upload files" on the top. You can multi-select all of the files that you want to upload to the current blob container.
* click on "Upload directory". You can select a whole folder to upload to the current blob container.

To refresh the blob list, press the "Refresh" button on the top.
