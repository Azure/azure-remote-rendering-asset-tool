---
title: Settings panel
description: Settings Panel in Azure Remote Rendering asset tool
author: mafranc
ms.author: mafranc
ms.date: 03/23/2020
ms.topic: article
---

# Settings panel

![Settings panel](media/settings.png)

To access the settings panel, click on the gear button on the top-right corner.

The panel allows you to change the following settings for ARRT:

* Remote Rendering account.
* Blob storage account. For instruction on how to set up your ARR and Blob storage accounts, see [Setting up your accounts](index.md#setting-up-your-accounts)
* Video streaming settings.
* Camera settings. Camera field of view, camera speeds, near/far field, global scale.
  * The global scale setting is applied at runtime to any loaded model. It's particularly useful when dealing with very large or very small models. With the "Auto" option the scale is automatically recomputed so that a loaded model always lies between the near and far plane.

> Note:
> If you are connected to a session and you modify the video streaming parameters, the changes will be applied at the next connection. For that to happen you can press the button "Apply" which will force a reconnection and unload any model that is currently loaded.

All of the changes are immediately applied after editing.

The configuration is persisted in a json file stored in *%LocalAppData%/Arrt/config.json*
