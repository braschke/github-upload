#!/bin/sh
exec "perl" -wSx $0 ${1+"$@"}
   if 0;
#!perl -w
#line 6
BEGIN {$ENV{'MPCCI_HOME'} = `mpcci home`}
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#
#  script to make/gmake the octbox library
#  Carsten Dehning, FhG SCAI,  March 2017
#  $Id: make.pl 462 2013-03-01 08:13:45Z dehning $
#
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
use 5.006;
use strict;
use File::Path;
use lib "$ENV{'MPCCI_HOME'}/perlmod";
use MpCCIBase;
use MpCCIArch;
use MpCCIEnv;
use MpCCICore::Which;
use MpCCIGui::Compilers;
use MpCCIMake::Makeutils;

sub _make_arch(@);


   mpcci_init();
   mpcci_arch();
   which_cc((IS_MSWIN&&!$ENV{'MAKE_MINGW'})?('MSVC',undef,1,1):'NATIVE');

   my ($startDir,$makeFile) = make_prepare(1);
   make_noopt();
   $ENV{'MAKE_CRTMD'} = 1 if (IS_MSWIN);

   if ($MPCCI_ARCH =~ /linux/i && $ENV{'MAKE_MINGW'})
   {
      _make_arch('mingw','linux_mingw_64','d',64);
      _make_arch('mingw','linux_mingw_64','s',64);
   }
   elsif ($MPCCI_ARCH =~ /windows/i && $ENV{'MAKE_MINGW'})
   {
      _make_arch('mingw','windows_mingw_64','d',64);
      _make_arch('mingw','windows_mingw_64','s',64);
   }
   elsif (IS_MSWIN64)
   {
      which_cc('MSVC',undef,1,1); # source 32 bit compiler
      _make_arch('windows_x86','windows_x86_32','d',32);
      _make_arch('windows_x86','windows_x86_32','s',32);
      which_cc('MSVC',undef,0,0); # source 64 bit compiler
      _make_arch($MPCCI_ARCH,$MPCCI_ARCH64,'d',64);
      _make_arch($MPCCI_ARCH,$MPCCI_ARCH64,'s',64);
   }
   elsif ($MPCCI_ARCH64)
   {
      _make_arch($MPCCI_ARCH,$MPCCI_ARCH64,'d',64);
      _make_arch($MPCCI_ARCH,$MPCCI_ARCH64,'s',64);
   }
   else
   {
      _make_arch($MPCCI_ARCH,$MPCCI_ARCH32,'d',32);
      _make_arch($MPCCI_ARCH,$MPCCI_ARCH32,'s',32);
   }
   make_closeup('lib');
exit(0);

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#
# run the make
#
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sub _make_arch(@)
{
   my ($dstArch,$makeArch,$prec,$bits) = @_;

   print "
***********************************************************************************
   ARCH: $makeArch
   PREC  $prec
   BITS: $bits
***********************************************************************************
";
   make_clean();
   make_run("-f $makeFile ARCH=$makeArch PREC=$prec BITS=$bits");
   for(which_files('*.a','*.lib'))
   {
     printf "$_\n";
     mpcci_fcopy($_,"$startDir/lib/$dstArch/$_");
   }
}

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
