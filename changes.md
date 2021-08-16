1.18.20
===




1.15.20
===

* Fixed `Y` jazz for negative inputs giving better scratching behaviour
* Fixed `D` to use oversampling
* Fixed `X` oversampling
* Calmed some of the high sizzle on `μ` (breaks compatibility in a good way) by a 4th order Linkwitz, seems a 3rd differential need a -24 dB on the top for a net -6 dB

1.15.19
===

* Added `X` oversampled wave folder
* Deleted `O` **.svg** component layer
* Alter `A` estimator to 4th order

1.14.19
===

* Faster `tanf` called `tanpif`
* Color lights on `Y`
* Jazz position input on `Y`
* Added MIDI `S` and ENV/VCA `E`
* Altered `μ` differential calculations to 4th order for less susceptability to Gibbs oscillations
* Altered `D` estimation calculation to 4th order

1.12.18
===

* `F` k cancels (breaks patch but is functioning as designed)
* `O` works

1.12.16
===

* Added oscillator `O`
* `F` Bessel coefficients from different source (breaks patch)
* `F` frequency fix

1.11.16
===

* Potential `Ω` sharing of display bug fixed (no effect on other function)
* Null terminal in data save `Ω`
* Added in `W` chromatic gate summator
* Added in `Y` gate pattern sequencer

1.9.15
===

* `F` tracks high pass pattern (breaks compatibility)
* `*-rc2` graphics fix colors

1.9.14
===

* Added `F` filter
* Removed `ERR` from `D` and added `FRQ` filter (breaks compatibility)

1.8.13
===

* Fixed version numbering mixup with 1.8.11
* Increased range of halflife on `μ` (breaks compatibility)
* Fixed unit `dBs` (relative log scaled seconds) on `μ` (breaks compatibility)

1.8.11
===

* LPF on `μ` added for slightly less noise on transients.


1.8.10
===

* Fixed percent controls on `Ω` (minor 1 in a million bug)
* Introduced unit `dBs` (relative log scaled seconds) on `V` (breaks compatibility)
* Introduced dB scaling on `A` for `REZ` and `AM` (breaks patch)
* Introduced unit `dBs` (relative log scaled seconds) on `μ` (breaks compatibility)

1.8.9
===

* Reduced scaling on `μ` input `CVdb` (breaks compatibility) 10V/6dB
* Improved documentation
* Website GitHub pages [Captain's log](https://jackokring.github.io/KRTPluginADemo/)

1.6.9
===

* Fixed issues of polypony
* Fixed time length of buffer `float` pointer in `L`
* Improved documentation
* Improved [demo repository](https://github.com/jackokring/KRTPluginADemo)
* Added cube root output scaling to `u` (breaks compatibility)
* Started change log (this file)

