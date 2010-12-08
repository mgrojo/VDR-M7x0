#!/usr/bin/perl

use strict;
use warnings;

my @LANGS = (
  "en_US",
  "de_DE",
  "sl_SI",
  "it_IT",
  "nl_NL",
  "pt_PT",
  "fr_FR",
  "nn_NO",
  "fi_FI",
  "pl_PL",
  "es_ES",
  "el_GR",
  "sv_SE",
  "ro_RO",
  "hu_HU",
  "ca_ES",
  "ru_RU",
  "hr_HR",
  "et_EE",
  "da_DK",
  "cs_CZ",
  "tr_TR"
  );

my %VERS = (
  "en_US" => 10200,
  "de_DE" => 10200,
  "sl_SI" => 10200,
  "it_IT" => 10200,
  "nl_NL" => 10200,
  "pt_PT" => 10200,
  "fr_FR" => 10200,
  "nn_NO" => 10200,
  "fi_FI" => 10200,
  "pl_PL" => 10200,
  "es_ES" => 10200,
  "el_GR" => 10200,
  "sv_SE" => 10200,
  "ro_RO" => 10200,
  "hu_HU" => 10200,
  "ca_ES" => 10200,
  "ru_RU" => 10302,
  "hr_HR" => 10307,
  "et_EE" => 10313,
  "da_DK" => 10316,
  "cs_CZ" => 10342,
  "tr_TR" => 10502
  );


my %strings;

foreach my $lang (@LANGS) { $strings{$lang} = { }; }


sub LoadLanguage(*) {
    my ($lang) = @_;
    
    if (!open FILE, "<", "po/$lang.po") {
        return 0;   
    }
    
    my $msgid = "";
    while (<FILE>) {
        chomp;
        my $line = $_;
        
        if ($line =~ /^msgid "(.*)"$/) {
            $msgid = $1;
        } elsif ($line =~ /^msgstr "(.*)"$/) {
            my $msgstr = $1;
            $strings{$lang}->{$msgid} = $msgstr;
            $strings{"en_US"}->{$msgid} = $msgid;
        }
    }
    
    close FILE;
}



foreach my $lang (@LANGS) { 
    LoadLanguage($lang);
}

my @msgids = sort keys %{$strings{"en_US"}};


my $silent = 0;

while (<>) {
    my $line = $_;

    if ($line =~ /^\/\/ START I18N/) {
        print "// START I18N - automatically generated by po2i18n.pl\n";
        for my $msgid (@msgids) {
            next if $msgid eq "";

            my $head = "  { ";
            my $endif = "";
            my $versnum = 10200;
            
            for my $lang (@LANGS) {
                if ($VERS{$lang} ne $versnum) {
                    $versnum = $VERS{$lang};
                    print $endif;
                    print "#if VDRVERSNUM >= $versnum\n";
                    $endif = "#endif\n";
                }
                my $msgstr = $strings{$lang}->{$msgid};
                $msgstr = "" if !defined $msgstr;
                
                print "$head\"$msgstr\",\n";
                $head = "    ";
            }
            print $endif;
            print "  },\n";
        }
        $silent = 1;
    }

    if (!$silent) { print $line; }

    if ($line =~ /^\/\/ END I18N/) { 
        print "// END I18N - automatically generated by po2i18n.pl\n";
        $silent = 0; 
    }    
}
