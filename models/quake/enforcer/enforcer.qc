$modelname "enforcer.mdl" 
$cd ".\" 
$cdtexture ".\" 
$scale 1.0 
$origin 0 0 -8 -90
$bodygroup body 
{ 
studio "reference_fixed" 
} 

$attachment 0 "joint34" 1 0 0
 
$sequence "stand" "stand" fps 2 loop ACT_IDLE 1
$sequence "walk" "walk" LX fps 10 loop ACT_WALK 1 { event 4 1 }
$sequence "run" "run" LX fps 10 loop ACT_RUN 1 { event 4 1 }
$sequence "attack" "attack" fps 10 ACT_RANGE_ATTACK1 1 { event 1 12 } { event 3 6 } { event 3 10 }
$sequence "death" "death" fps 10 ACT_DIESIMPLE 1 { event 2 4 }
$sequence "fdeath" "fdeath" fps 10 ACT_DIESIMPLE 1 { event 2 4 }
$sequence "paina" "paina" fps 10 ACT_BIG_FLINCH 1
$sequence "painb" "painb" fps 10 ACT_BIG_FLINCH 1
$sequence "painc" "painc" fps 10 ACT_BIG_FLINCH 1 
$sequence "paind" "paind" fps 10 ACT_BIG_FLINCH 1
