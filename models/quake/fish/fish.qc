$modelname "fish.mdl" 
$cd ".\" 
$cdtexture ".\" 
$scale 1.0 
$origin 0 0 0 -90

$bodygroup body 
{ 
studio "reference" 
} 

$sequence "swim" "aswim" fps 10 loop ACT_SWIM 1 { event 1 1 }
$sequence "pain" "pain" fps 10 ACT_BIG_FLINCH 1
$sequence "death" "death" fps 10 ACT_DIESIMPLE 1
$sequence "attack" "attack" fps 10 ACT_MELEE_ATTACK1 1 { event 2 3 } { event 2 9 } { event 2 15 }
