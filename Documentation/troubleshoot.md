---
title: ARRT Troubleshooting
description: Lists known issues and potential solutions
author: jakras
ms.author: jakras
ms.date: 06/07/2022
ms.topic: article
---

# ARRT Troubleshooting

## Network Proxies

ARRT does not support to set a network proxy. However, this can be configured on the user's machine directly.

In an **admin command prompt** run:

```cmd
netsh winhttp show proxy
```

Check whether a proxy is already configured. If so, and connections still don't work, see [connection to ARR sessions fail](#connection-to-arr-sessions-fail) below.

If another proxy is already set, write it down so that you can restore it later.

To set a new proxy run:

```cmd
netsh winhttp set proxy 10.0.0.6:8080 (fill in your proxy)
```

Please note that this potentially influences all programs which use winhttp internally.

Once you are done using ARRT, you can reset the proxy with:

```cmd
netsh winhttp reset proxy
```

Or, if you already had another proxy before, use the set proxy command to revert the setting to that instead.

## Connections to ARR sessions fail

Connections to remote rendering sessions use plain UDP/TCP and therefore certain ports must be allowed through the firewall. Sometimes firewalls block these directly on the user's machine and sometimes firewalls inside routers are configured to block them. Make sure to check both.

See the [ARR system requirements](https://docs.microsoft.com/azure/remote-rendering/overview/system-requirements#network-firewall) for the IP ranges and ports that have to be allowed through.
