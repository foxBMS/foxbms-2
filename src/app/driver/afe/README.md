# Build Script Instructions

- At lowest level an AFE library shall be linked (e.g., for the LTC LTC6813-1,
  the library shall be linked in `src/app/driver/afe/ltc/6813-1/wscript`).
- The target name shall be `f"foxbms-afe"`.
- Internally each library can use libraries as needed, as long as the above
  mentioned AFE library is built.
- This AFE library is then used in when linking the driver library
  `f"foxbms-driver"` at `src/app/driver/wscript`.
