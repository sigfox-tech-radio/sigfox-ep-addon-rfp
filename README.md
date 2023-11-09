# Sigfox End-Point RF & Protocol addon (EP_ADDON_RFP)

## Description

The **Sigfox End-Point RF & Protocol addon** shows an example of the [RF & Protocol test](https://support.sigfox.com/docs/rf-protocol-test-specification) implementation. This addon is an application of the [Sigfox End-Point library](https://github.com/sigfox-tech-radio/sigfox-ep-lib) which offers a new API (provided in the `sigfox_ep_addon_rfp_api.h` file) to execute Sigfox test modes.

The table below shows the versions compatibility between this addon and the Sigfox End-Point library.

| **EP_ADDON_RFP** | **EP_LIB** |
|:---:|:---:|
| [v1.4](https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp/releases/tag/v1.4) | >= [v3.2](https://github.com/sigfox-tech-radio/sigfox-ep-lib/releases/tag/v3.2) |
| [v1.3](https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp/releases/tag/v1.3) | >= [v3.2](https://github.com/sigfox-tech-radio/sigfox-ep-lib/releases/tag/v3.2) |
| [v1.2](https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp/releases/tag/v1.2) | >= [v3.2](https://github.com/sigfox-tech-radio/sigfox-ep-lib/releases/tag/v3.2) |
| [v1.1](https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp/releases/tag/v1.1) | [v3.1](https://github.com/sigfox-tech-radio/sigfox-ep-lib/releases/tag/v3.1) |
| [v1.0](https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp/releases/tag/v1.0) | [v3.0](https://github.com/sigfox-tech-radio/sigfox-ep-lib/releases/tag/v3.0) |

## Stack architecture

<p align="center">
<img src="https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp/wiki/images/sigfox_ep_addon_rfp_architecture.drawio.png" width="600"/>
</p>

## Compilation flags for optimization

This addon inherits all the [Sigfox End-Point library flags](https://github.com/sigfox-tech-radio/sigfox-ep-lib/wiki/compilation-flags-for-optimization) and can be optimized accordingly.

The `CERTIFICATION` flag must be enabled to use this addon.

## How to add Sigfox RF & Protocol addon to your project

### Dependencies

The only dependency of this addon is the [Sigfox End-Point library](https://github.com/sigfox-tech-radio/sigfox-ep-lib) source code.

### Submodule

The best way to embed the Sigfox End-Point RF & Protocol addon into your project is to use a [Git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules), in a similar way to the library. The addon will be seen as a sub-repository with independant history. It will be much easier to **upgrade the addon** or to **switch between versions** when necessary, by using the common `git pull` and `git checkout` commands within the `sigfox-ep-addon-rfp` folder.

To add the Sigfox RF & Protocol addon submodule, go to your project location and run the following commands:

```bash
mkdir lib
cd lib/
git submodule add https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp.git
```

This will clone the Sigfox End-Point RF & Protocol addon repository. At project level, you can commit the submodule creation with the following commands:

```bash
git commit --message "Add Sigfox End Point RF and Protocol addon submodule."
git push
```

With the submodule, you can easily:

* Update the addon to the **latest version**:

```bash
cd lib/sigfox-ep-addon-rfp/
git pull
git checkout master
```

* Use a **specific release**:

```bash
cd lib/sigfox-ep-addon-rfp/
git pull
git checkout <tag>
```

### Raw source code

You can [download](https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp/releases) or clone any release of the Sigfox End-Point RF & Protocol addon and copy all files into your project.

```bash
git clone https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp.git
```

### Precompiled source code

You can [download](https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp/releases) or clone any release of the Sigfox End-Point RF & Protocol addon and copy all files into your project. If you do not plan to change your compilation flags in the future, you can perform a **precompilation step** before copying the file in your project. The precompilation will **remove all preprocessor directives** according to your flags selection, in order to produce a more **readable code**. Then you can copy the new files into your project.

```bash
git clone https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp.git
```

To perform the precompilation, you have to install `cmake` and `unifdef` tools, and run the following commands:

```bash
cd sigfox-ep-addon-rfp/
mkdir build
cd build/
```

* Precompiling by reading the `sigfox_ep_flags.h` file:

```bash
cmake -DSIGFOX_EP_LIB_DIR=<sigfox-ep-lib path> \
      -DUSE_SIGFOX_EP_FLAGS_H=ON ..
make precompil_sigfox_ep_addon_rfp
```

* Precompiling by entering the flags selection on command line:

```bash
cmake -DSIGFOX_EP_LIB_DIR=<sigfox-ep-lib path> \
      -DUSE_SIGFOX_EP_FLAGS_H=OFF \
      -DRC1=ON \
      -DRC2=ON \
      -DRC3C=ON \
      -DRC3D=ON \
      -DRC4=ON \
      -DRC5=ON \
      -DRC6=ON \
      -DRC7=ON \
      -DAPPLICATION_MESSAGES=ON \
      -DCONTROL_KEEP_ALIVE_MESSAGE=ON \
      -DBIDIRECTIONAL=ON \
      -DASYNCHRONOUS=ON \
      -DLOW_LEVEL_OPEN_CLOSE=ON \
      -DREGULATORY=ON \
      -DLATENCY_COMPENSATION=ON \
      -DSINGLE_FRAME=ON \
      -DUL_BIT_RATE_BPS=OFF \
      -DTX_POWER_DBM_EIRP=OFF \
      -DT_IFU_MS=OFF \
      -DT_CONF_MS=OFF \
      -DUL_PAYLOAD_SIZE=OFF \
      -DCRC_HW=OFF \
      -DMESSAGE_COUNTER_ROLLOVER=OFF \
      -DPARAMETERS_CHECK=ON \
      -DCERTIFICATION=ON \
      -DPUBLIC_KEY_CAPABLE=ON \
      -DVERBOSE=ON \
      -DERROR_CODES=ON \
      -DERROR_STACK=12 ..
make precompil_sigfox_ep_addon_rfp
```

The new files will be generated in the `build/precompil` folder.

### Static library

You can also [download](https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp/releases) or clone any release of the Sigfox End-Point RF & Protocol addon and build a **static library**.

```bash
git clone https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp.git
```

To build a static library, you have to install `cmake` tool and run the following commands:

```bash
cd sigfox-ep-addon-rfp/
mkdir build
cd build/
```

* Building by reading the `sigfox_ep_flags.h` file:

```bash
cmake -DSIGFOX_EP_LIB_DIR=<sigfox-ep-lib path> \
      -DUSE_SIGFOX_EP_FLAGS_H=ON ..
make sigfox_ep_addon_rfp
```

* Building by entering the flags selection on command line:

```bash
cmake -DSIGFOX_EP_LIB_DIR=<sigfox-ep-lib path> \
      -DUSE_SIGFOX_EP_FLAGS_H=OFF \
      -DRC1=ON \
      -DRC2=ON \
      -DRC3C=ON \
      -DRC3D=ON \
      -DRC4=ON \
      -DRC5=ON \
      -DRC6=ON \
      -DRC7=ON \
      -DAPPLICATION_MESSAGES=ON \
      -DCONTROL_KEEP_ALIVE_MESSAGE=ON \
      -DBIDIRECTIONAL=ON \
      -DASYNCHRONOUS=ON \
      -DLOW_LEVEL_OPEN_CLOSE=ON \
      -DREGULATORY=ON \
      -DLATENCY_COMPENSATION=ON \
      -DSINGLE_FRAME=ON \
      -DPARAMETERS_CHECK=ON \
      -DCERTIFICATION=ON \
      -DPUBLIC_KEY_CAPABLE=ON \
      -DVERBOSE=ON \
      -DCRC_HW=OFF \
      -DERROR_CODES=ON \
      -DUL_BIT_RATE_BPS=OFF \
      -DT_IFU_MS=OFF \
      -DT_CONF_MS=OFF \
      -DUL_PAYLOAD_SIZE=OFF \
      -DMESSAGE_COUNTER_ROLLOVER=OFF \
      -DERROR_STACK=12 ..
make sigfox_ep_addon_rfp
```

The archive will be generated in the `build/lib` folder.
