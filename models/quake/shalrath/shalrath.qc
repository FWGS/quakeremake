$modelname "shalrath.mdl" 
$cd ".\" 
$cdtexture ".\" 
$scale 1.0 
$origin 0 0 -15 -90 
$bodygroup body 
{ 
studio "reference" 
} 
 
$sequence "idle" "idle" fps 10 loop ACT_IDLE 1
$sequence "walk" "walk" LX fps 10 loop ACT_WALK 1 { event 1 1 }
$sequence "run" "walk" LX fps 15 loop ACT_RUN 1 { event 1 1 }
$sequence "attack" "attack" fps 10 ACT_MELEE_ATTACK1 1 { event 2 8 } { event 3 1 }
$sequence "death" "death" fps 10 ACT_DIESIMPLE 1
$sequence "pain" "pain" fps 10 ACT_BIG_FLINCH 1
