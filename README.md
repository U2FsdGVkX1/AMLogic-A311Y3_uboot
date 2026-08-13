# Amlogic A311Y3 U-Boot

U-Boot source and firmware components for the Amlogic A311Y3 BY401 board.
The default configuration is `a9_by401`.

## Build

Install an AArch64 cross compiler and the host tools required by U-Boot and
the Amlogic packaging scripts. On Debian or Ubuntu, the required packages
include `gcc-aarch64-linux-gnu`, `make`, `git`, `zip`, `python3`, `openssl`,
`xxd`, `bison`, and `flex`.

Run:

```sh
CONFIG_BYPASS_AOCPU=y \
CROSS_COMPILE=aarch64-linux-gnu- \
ARCH=arm \
./mk a9_by401 --disable-bl33z \
  --bl30 bl30/bin_ao/a9/a311y3/bl30.bin
```

`CONFIG_BYPASS_AOCPU=y` uses the included, prebuilt A311Y3 BL30 image and
avoids requiring the vendor RISC-V toolchain. `--disable-bl33z` skips the
optional ramdump companion image.

Successful builds produce these files under `build/`:

- `u-boot.bin.signed`
- `u-boot.bin.sd.bin.signed`
- `u-boot.bin.usb.signed`
- `a9_by401-u-boot.aml.zip`

Build outputs are intentionally excluded from this repository.

## BL30

The prebuilt BL30 firmware used by the verified build is:

```text
bl30/bin_ao/a9/a311y3/bl30.bin
SHA256: b9c653bf5d673256cda50aab3319df4bba8752a812ffc7a38e297dabc81d33d1
```

## Boot Flow Changes

- Uses standard bootflow scanning with USB before eMMC/SD and network targets.
- Initializes USB during `preboot` so USB-hosted systems can be discovered.
- Applies Amlogic reserved-memory checks before installing an external EFI DTB.

