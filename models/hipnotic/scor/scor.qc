$modelname "../scor.mdl" 
$cd ".\" 
$cdtexture ".\" 
$scale 1.0 
$origin 0 0 7 -90 
$bodygroup body 
{ 
studio "reference" 
} 

$attachment 0 "joint47" 3 0 0
$attachment 1 "joint53" 3 0 0

$sequence "stand" "stand" loop fps 5 ACT_IDLE 1
$sequence "walk" "walk" LX loop fps 8 ACT_WALK 1 { event 1 1 }
$sequence "run" "walk" LX loop fps 12 ACT_RUN 1 { event 1 1 } { event 2 2 }
$sequence "strafe_left" "walk" LX loop fps 8 ACT_STRAFE_LEFT 1
$sequence "strafe_right" "walk" LX loop fps 8 ACT_STRAFE_RIGHT 1
$sequence "death" "death" fps 10 ACT_DIESIMPLE 1
$sequence "pain" "pain" fps 10 ACT_BIG_FLINCH 1
$sequence "shoot" "shoot" fps 10 ACT_RANGE_ATTACK1 1 { event 5001 1 "11" } { event 5011 2 "11" } { event 5001 3 "11" } { event 5011 4 "11" } { event 4 1 } { event 5 2 } { event 4 3 } { event 5 4 }
$sequence "sting" "sting" fps 10 ACT_MELEE_ATTACK1 1 { event 3 5 }