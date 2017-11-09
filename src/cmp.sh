#! /bin/csh
#++++++++++++++++
#.IDENTIFICATION cmp.sh
#.LANGUAGE       C-shell
#.AUTHOR         Francois Ochsenbein [CDS]
#.ENVIRONMENT    
#.KEYWORDS       
#.VERSION  1.0   28-Apr-1999
#.PURPOSE        
#.COMMENTS       Compare GSC 1.1 & 2
#----------------

rm -f 1.1 1.2

@ d = 90
while ($d >= 0)
    @ da = 3
    if ($d >= 45) @ da = 5
    if ($d >= 75) @ da = 15
    if ($d >= 85) @ da = 45
    @ a = 0
    while ($a < 360)
	setenv GSCDAT /GSC
	/GSC/bin/gsc.exe -n 1000 -c $a.0+$d.0 -r 20 > 1.1
	setenv GSCDAT .
	     bin/gsc.exe -n 1000 -c $a.0+$d.0 -r 20 > 1.2
	set n1 = `wc -l < 1.1`
	set n2 = `wc -l < 1.2`
	diff 1.1 1.2 | gawk -v a=$a -v d=$d -v n1=$n1 -v n2=$n2 'BEGIN{\
	    printf("----Center %03d+%02d: %4d(1.1) %4d(1.2)\n",a,d,n1,n2)} \
	    { print "    " $0}' 
	rm 1.1 1.2

	setenv GSCDAT /GSC
	/GSC/bin/gsc.exe -n 1000 -c $a.0-$d.0 -r 20 > 1.1
	setenv GSCDAT .
	     bin/gsc.exe -n 1000 -c $a.0-$d.0 -r 20 > 1.2
	set n1 = `wc -l < 1.1`
	set n2 = `wc -l < 1.2`
	diff 1.1 1.2 | gawk -v a=$a -v d=$d -v n1=$n1 -v n2=$n2 'BEGIN{\
	    printf("----Center %03d-%02d: %4d(1.1) %4d(1.2)\n",a,d,n1,n2)} \
	    { print "    " $0}' 
	rm 1.1 1.2
	@ a += $da
    end
    @ d -= 3
end
