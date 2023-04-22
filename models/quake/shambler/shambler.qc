$modelname "shambler.mdl"
$cd ".\"
$cdtexture ".\"
$scale 1.0
$cliptotextures
$origin 0 0 24 0

$bbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$cbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$eyeposition 15.000000 0.000000 90.000000


//reference mesh(es)
$body "body" "ref_shambler"


// 2 attachment(s)
$attachment 0 "Bip01 L Hand" 0.000000 0.000000 0.000000
$attachment 1 "Bip01 R Hand" 0.000000 0.000000 0.000000

// 2 bone controller(s)
$controller 0 "Bip01 Head" ZR -30.000000 30.000000
$controller 1 "Bip01 Head" XR -30.000000 30.000000

// 29 hit box(es)
$hbox 0 "Bip01 Pelvis" -10.963243 -0.500004 -0.000005 9.963253 2.893433 2.115075
$hbox 0 "Bip01 L Thigh" -10.290000 -9.160000 -8.680000 6.570000 14.150000 18.045784
$hbox 0 "Bip01 L Calf" -5.940000 -9.890000 -2.730000 9.430000 8.040000 21.716688
$hbox 0 "Bip01 L Foot" -8.830000 -10.890000 -1.410000 7.970000 11.140000 6.160000
$hbox 0 "Bip01 R Thigh" -9.640000 -13.510000 -8.440000 6.910000 8.380000 18.045719
$hbox 0 "Bip01 R Calf" -6.800000 -9.770000 -6.040000 7.910000 6.040000 21.765577
$hbox 0 "Bip01 R Foot" -10.100000 -15.360000 -1.860000 7.300000 5.160000 4.160000
$hbox 0 "Bip01 Spine1" -11.300000 0.000000 -4.440000 10.160000 17.180000 9.913568
$hbox 0 "Bip01 Spine3" -19.389999 -1.820000 -22.809999 19.370001 6.460000 6.662395
$hbox 0 "Bip01 Neck" -20.150000 -14.910000 -8.860000 17.959999 16.510000 7.220000
$hbox 0 "Shamhead" -10.040000 -12.700000 -5.160000 12.140000 11.990000 7.380000
$hbox 0 "Bip01 L UpperArm" -7.860000 -5.730000 -2.000000 9.580000 7.250000 17.786898
$hbox 0 "Bip01 L Forearm" -8.970000 -6.790000 0.000000 8.000000 7.450000 32.973148
$hbox 0 "Bip01 L Hand" -4.197182 -4.290000 0.000000 6.165050 3.590000 6.469132
$hbox 0 "Bip01 R UpperArm" -8.370000 -7.570000 -6.310000 6.080000 7.810000 17.953350
$hbox 0 "Bip01 R Forearm" -6.280000 -7.340000 0.000000 7.000000 6.360000 29.372391
$hbox 0 "Bip01 R Hand" -5.748405 -6.073123 0.000000 5.121327 3.150000 8.973324
$hbox 0 "Bip01 R Finger0" -2.570000 -4.150000 -1.310000 3.000000 2.660000 5.317405
$hbox 0 "Bip01 R Finger1" -2.260000 -2.930000 -3.710000 3.190000 2.210000 4.240919
$hbox 0 "Bip01 R Finger2" -2.950000 -2.990000 -2.670000 1.400000 1.620000 5.481764
$hbox 0 "joint27" -2.800000 -1.780000 -2.080000 1.860000 3.120000 9.100000
$hbox 0 "joint28" -1.470000 -3.180000 -2.220000 2.860000 2.120000 10.540000
$hbox 0 "joint29" -1.750000 -1.710000 -4.610000 2.350000 1.640000 9.140000
$hbox 0 "joint30" -3.400000 -2.290000 -0.420000 2.680000 1.720000 4.728693
$hbox 0 "joint31" -1.570000 -1.320000 0.000000 2.660000 5.800000 11.580000
$hbox 0 "joint32" -4.080000 -1.850000 -1.470000 2.140000 3.220000 5.234588
$hbox 0 "joint33" -2.730000 -2.440000 -2.320000 4.120000 2.650000 11.590000
$hbox 0 "joint34" -2.850000 -1.680000 0.000000 2.460000 4.050000 3.883958
$hbox 0 "joint35" -3.370000 -11.310000 -0.760000 1.230000 0.380000 8.950000

// 8 animation sequence(s)
$sequence "stand" "stand" fps 10 loop ACT_IDLE 1 
$sequence "walk" "walk_lx" fps 10 LX loop ACT_WALK 1 { event 1 12 } { event 2 6 } { event 3 12 }   
$sequence "run" "run_lx" fps 10 LX loop ACT_RUN 1 { event 1 5 } { event 2 1 } { event 3 4 }    
$sequence "smash" "smash" fps 10 ACT_MELEE_ATTACK1 1 { event 10 1 } { event 7 6 } 
$sequence "swing" "swing" fps 10 ACT_MELEE_ATTACK1 1 { event 11 1 } { event 8 5 }  
$sequence "magic" "magic" fps 10 ACT_RANGE_ATTACK1 1 { event 5 2 } { event 6 3 } { event 4 4 } { event 4 5 } { event 4 6 } { event 4 7 }
$sequence "pain" "pain" fps 10 ACT_BIG_FLINCH 1 
$sequence "death" "death" fps 10 ACT_DIESIMPLE 1

// End of QC script.
