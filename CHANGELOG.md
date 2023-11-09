# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [v1.4](https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp/releases/tag/v1.4) - 09 Nov 2023

### Removed

* Remove `doc` folder since images are now hosted on the GitHub wiki.

### Changed

* Change `tests_mode` directory name in to `test_modes_rfp`

### Known limitations

* **Test mode G** not supported.
* **Test mode H** not supported.
* **Test mode I** not supported.

## [v1.3](https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp/releases/tag/v1.3) - 10 Aug 2023

### Changed

* Change **Test mode F** , t_conf timer = 2000ms.  
* Change **Test mode C** , always in frame rank = 1.

### Known limitations

* **Test mode G** not supported.
* **Test mode H** not supported.
* **Test mode I** not supported.

## [v1.2](https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp/releases/tag/v1.2) - 16 Jun 2023

### Changed

* Replace -DSFX_LIB_CORE_PROTOCOL_DIR by -DSIGFOX_EP_LIB_DIR in cmake file.  
* Add field name of anonymous structures  according to the library.
* Replace TX control flags naming in test API according to the library.
* Remove types warnings (unsigned to sfx_u8).
* Change status naming according to the library.

### Known limitations

* **Test mode G** not supported.
* **Test mode H** not supported.
* **Test mode I** not supported.

## [v1.1](https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp/releases/tag/v1.1) - 01 Mar 2023

### General

* Add `CHANGELOG` file.

### Added

* Add IFU timer between test mode C frames
* Using the new **bypass flags** in test API
* Implement **Test mode D**.
* Implement **Test mode E**.
* Implement **Test mode F**.

### Changed

* Disable REGULATORY check for some tests mode and reduce test time.
* Rename multi inclusion preprocessor directive  
* Set `ul_payload` to NULL when sending empty/bit0/bit1 frame in **Test mode F**
* Random frequency for **Test mode K**
* Do no call `MCU_API_timer_stop` when  `MCU_TIMER_start` failed
* Add  `MCU_API_TIMER_REASON_ADDON_RFP` reason for all in timer started by this addon.


### Known limitations

* **Test mode G** not supported.
* **Test mode H** not supported.
* **Test mode I** not supported.

## [v1.0](https://github.com/sigfox-tech-radio/sigfox-ep-addon-rfp/releases/tag/v1.0) - 12 Dec 2022

### General

* First version of the Sigfox End-Point RF & Protocol addon.

### Added

* **Asynchronous** operation mode in addition to the legacy blocking mode.
* **Memory footprint optimization** thanks to multiple **compilation flags** (`inc/sigfox_ep_flags.h`)
* **Precompiled source** files and **library** generation with `cmake`.
* Using **structures** as functions parameters to improve compatibility between compilation flags and versions.
* **Test mode A** .
* **Test mode B** .
* **Test mode C** .
* **Test mode K** .
* **Test mode L** .
* **Test mode J** .


### Known limitations

* **Test mode D** not supported.
* **Test mode E** not supported.
* **Test mode F** not supported.
* **Test mode G** not supported.
* **Test mode H** not supported.
* **Test mode I** not supported.
