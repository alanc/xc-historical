#ifdef macII
#define ccflags "-DmacII"
#endif /* macII */

#ifdef hpux
#define ccflags "-Wc,-Nd4000,-Ns3000 -DSYSV"
#endif /* hpux */

#ifndef ccflags
#define ccflags "-O"
#endif /* ccflags */

main()
{
	write(1, ccflags, sizeof(ccflags) - 1);
	exit(0);
}
