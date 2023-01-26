2.26.28
===

* TBC - `Z`
* default bypass on some modules?
* `json_object_set` replaced by `json_object_set_new` memory leak fix
* PayPal donate as GitHub use a bank which ignores Companies House details
* Potential vector optimization
* Fixed `U` independant polyphonic
* Left `V` as some hackey normaled mess
* Fixed `E` modulation bug
* Fixed `E`, `V` envelope bug
* Depricated `V`

2.25.27
===

* complete V2 migration labellings
* improve tool tip documentation

2.25.26
===

* made this file link here ?
* increased SDK V2 (2.25.25) but this file was misdirected
* started adding some V2 labellings to some things
* made `README.md` include version design in index
* decided to follow lowercase convention for labels

## Version 1 SDK Defunct

1.25.25 (actually shows as 1.25.24)
===

* fixed initialization bug regression for some modules added in 1.23.24
* extended control ranges on `G`
* forgot to bump version number so 1.25.24 published is actually 1.25.25
* Final SDK V1 version

1.25.24 (an main build unpublished version number)
===

* clocking on `S` fixed MIDI arm style
* reset on high in `Ω`
* added `I` clock divider
* added `G` compressor

1.23.24
===

* filter zero initializations (all modules)
* increased mash range on `X`

1.23.23
===

* set defualt `J` mix at 50%
* added `H` phase modulable organ

1.22.22
===

* individual DC filters on `B` (can break patch as now individual filters)
* added `J` all-pass filter/phaser
* added some relevant tags in .json
* added `K` phase modulation oscillator


1.20.21
===

* Made `C` gain normal not SD scaled for 2 ins (via mix) and 1 in split scaling (breaks patch)
* Fixed `C` color on mix label
* Reverted `μ` weighted average drift (fixes patch, mush on 2 of outputs fixed)
* Added `U` sample and hold quantizer
* Added `B` performance router

1.18.21
===

* Fixed `Q` singularity clicks (breaks patch)

1.18.20
===

* Fixed `F` damping as sqrt*sqrt gives correct evaluation of damping and also introduces a sample and hold behaviour on high modulation levels in the "full on" corner (slightly breaks patch)
* Added `M` module for base pre-emphisis or RIAA deck preamp with inverse "cutting metal" filter (many VCO can't reach a 20KHz sine high and matching drifts a little)
* Added `Q` module for some effect of a "quantum gravity following" of a source signal
* Added `μ` weighted average drift acceleration algorithm
* Added `C` module for mixing

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
