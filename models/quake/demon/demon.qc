$modelname "demon.mdl" 
$cd ".\" 
$cdtexture ".\" 
$scale 1.0 
$origin 0 0 4 -90 

$bodygroup body 
{ 
studio "reference" 
} 

$sequence "stand" "stand" loop fps 10 ACT_IDLE 1
$sequence "walk" "walk" LX loop fps 10 ACT_WALK 1 { event 1 1 }
$sequence "run" "run" LX loop fps 10 ACT_RUN 1 { event 1 1 }
$sequence "leap" "leap" fps 10 ACT_LEAP 1 { event 2 2 } 
$sequence "attacka" "attacka" fps 10 ACT_MELEE_ATTACK1 1 { event 3 5 } { event 4 10 } 
$sequence "pain" "pain" fps 10 ACT_BIG_FLINCH 1
$sequence "death" "death" fps 10 ACT_DIESIMPLE 1 