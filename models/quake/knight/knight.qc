$modelname "knight.mdl" 
$cd ".\" 
$cdtexture ".\" 
$scale 1.0 
$origin 0 0 -3 -90

$bodygroup body 
{ 
studio "reference" 
} 


 
$sequence "stand" "stand" fps 10 loop ACT_IDLE 1
$sequence "standing" "standing" loop fps 10
$sequence "walk" "walk" LX fps 10 loop ACT_WALK 1 { event 3 1 }
$sequence "runb" "runb" LX fps 10 loop ACT_RUN 1 { event 3 1 }
$sequence "runattack" "runattack" LX fps 10 ACT_MELEE_ATTACK2 1 { event 1 4 } { event 2 9 } 

$sequence "attackb" "attackb" fps 10 ACT_MELEE_ATTACK1 1 { event 1 5 } { event 2 8 } 
$sequence "death" "death" fps 10 ACT_DIESIMPLE 1
$sequence "deathb" "deathb" fps 10 ACT_DIESIMPLE 1
$sequence "kneel" "kneel" fps 10
$sequence "pain" "pain" fps 10 ACT_BIG_FLINCH 2
$sequence "painb" "painb" fps 10 ACT_BIG_FLINCH 1
