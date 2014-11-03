#!/bin/csh -f
set l = `ipcs -m | grep "$USER"| cut -c10-19`
foreach s ( $l )
    echo $s
    ipcrm -M $s 
end
if ($#l != 0 ) echo $#l shared memory\(s\) for $user removed
