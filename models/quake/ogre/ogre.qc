$modelname "ogre.mdl" 
$cd ".\" 
$cdtexture ".\" 
$scale 1.0 
$origin 0 0 -7 -90

$bodygroup body 
{ 
studio "reference" 
} 

$sequence "stand" "stand" loop fps 2 ACT_IDLE 1 { event 2 5 }
$sequence "walk" "walk" LX loop fps 10 ACT_WALK 1 { event 2 4 } { event 4 8 }
$sequence "run" "run" LX loop fps 10 ACT_RUN 1 { event 3 1 }
$sequence "death" "death" fps 10 ACT_DIESIMPLE 1 { event 1 6 }
$sequence "bdeath" "bdeath" fps 10 ACT_DIESIMPLE 1 { event 1 4 }
$sequence "smash" "smash" fps 10 ACT_MELEE_ATTACK1 1 { event 7 1 } { event 6 6 "0" } { event 6 7 "0" } { event 6 8 "0" } { event 6 9 "0" } { event 6 10 "1" } { event 6 11 "0" }
$sequence "swing" "swing" fps 10 ACT_MELEE_ATTACK1 1 { event 7 1 } { event 6 6 "0" } { event 6 7 "200" } { event 6 8 "0" } { event 6 9 "0" } { event 6 10 "-200" } { event 6 11 "0" }
$sequence "pull" "pull" fps 10
$sequence "shoot" "shoot" fps 10 ACT_RANGE_ATTACK1 1 { event 5 3 }
$sequence "pain" "pain" fps 10 ACT_BIG_FLINCH 1
$sequence "painb" "painb" fps 10 ACT_BIG_FLINCH 1
$sequence "painc" "painc" fps 10 ACT_BIG_FLINCH 1
$sequence "paind" "paind" fps 10 ACT_BIG_FLINCH 1
$sequence "paine" "paine" fps 10 ACT_BIG_FLINCH 1
