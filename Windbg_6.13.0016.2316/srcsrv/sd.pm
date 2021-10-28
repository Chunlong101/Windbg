# -----------------------------------------------------------------------------
# sd.pm
#
# Copyright (c) 2004-2006  Microsoft Corporation
#
# Source Server indexing module for Source Depot
# -----------------------------------------------------------------------------

package SD;

require Exporter;
use strict;

my %EXPORT_TAGS = ( 'all' => [ qw() ] );
my @EXPORT_OK   = ( @{ $EXPORT_TAGS{'all'} } );
my @EXPORT      = qw();
my $VERSION     = '0.1';

# -----------------------------------------------------------------------------
# Simple subs to make it clear when we're testing for BOOL values
# -----------------------------------------------------------------------------
sub TRUE   {return(1);} # BOOLEAN TRUE
sub FALSE  {return(0);} # BOOLEAN FALSE

# -----------------------------------------------------------------------------
# Create a new blessed reference that will maintain state for this instance of
# indexing
# -----------------------------------------------------------------------------
sub new {
    my $proto = shift;
    my $class = ref($proto) || $proto;
    my $self  = {};
    bless($self, $class);

    #
    # The command to use for talking to the server.  We don't allow this
    # to be overridden at the command line.
    #
    if ( defined $ENV{'SD_CMD'} ) {
        $$self{'SD_CMD'} = $ENV{'SD_CMD'};
    } else {
        $$self{'SD_CMD'} = "sd.exe";
    }

    $$self{'SD_LABEL'}     = $ENV{'SD_LABEL'}
        if ( defined $ENV{'SD_LABEL'}     );

    $$self{'SD_DEBUGMODE'} = 0;

    # Block for option parsing.
    PARSEOPTIONS: {
        my @unused_opts;
        my @opt;
        foreach (@ARGV) {
            # handle command options
            if (substr($_, 0, 1) =~ /^[\/-]$/) {
                # options that set values
                if ( (@opt = split(/=/, $_))==2 ) {
                    block: {
                        $$self{'SD_CMD'}     = $opt[1], last
                                if ( uc substr($opt[0], 1) eq "CMD");
                        $$self{'SD_LABEL'}   = $opt[1], last
                                if ( uc substr($opt[0], 1) eq "LABEL");
                        $$self{'SD_NEWROOT'} = $opt[1], last
                                if ( uc substr($opt[0], 1) eq "NEWROOT");
                        $$self{'SD_OLDROOT'} = $opt[1], last
                                if ( uc substr($opt[0], 1) eq "OLDROOT");
                        # Remember this was unused
                        push(@unused_opts, $_);
                        1;
                    }

                # options that are just flags
                } else {
                    block: {
                        $$self{'SHOWCMDS'}   = TRUE, last
                                if ( uc substr($opt[0], 1) eq "SHOWCMDS");
                        # Remember this was unused
                        push(@unused_opts, $_);
                        1;
                    }
                }
            } else {
                # Remember this was unused
                push(@unused_opts, $_);
            }
        }

        # Fixup @ARGV to only contained unused options so SSIndex.cmd
        # can warn the user about them if necessary.
        @ARGV = @unused_opts;
    }

    $$self{'FILE_LOOKUP_TABLE'} = ();

    return($self);
}

# -----------------------------------------------------------------------------
# Display module internal option state.
# -----------------------------------------------------------------------------
sub DisplayVariableInfo {
    my $self = shift;

    ::status_message("%-15s: %s\n",
                     "SD program name",
                     $$self{'SD_CMD'});

    ::status_message("%-15s: %s\n",
                    "SD Label",
                    $$self{'SD_LABEL'} ? $$self{'SD_LABEL'}      : "<N/A>");

    ::status_message("%-15s: %s\n",
                     "Old path root",
                     $$self{'SD_OLDROOT'} ? $$self{'SD_OLDROOT'} : "<N/A>");

    ::status_message("%-15s: %s\n",
                     "New path root",
                     $$self{'SD_NEWROOT'} ? $$self{'SD_NEWROOT'} : "<N/A>");

}

# -----------------------------------------------------------------------------
# Given our init data and a local source path, create a lookup table that can
# return individual stream data for each source file.
# -----------------------------------------------------------------------------
sub GatherFileInformation {
    my $self       = shift;
    my $SourceRoot = shift;
    my $ServerRefs = shift;
    my $ExcludesRef= shift;

    my %FileLookup;
    my $hProc;

    # Remember the current directory
    my $PrevDir    = `cd`;
    chomp $PrevDir;

    chdir($SourceRoot);

    my ($Server, $Root, $PreferredAlias, $PreferredServer);

    if ( defined $$self{'SHOWCMDS'} && $$self{'SHOWCMDS'} ) {
        ::status_message("[CWD: $SourceRoot] Running \"$$self{'SD_CMD'} info\"\n");
    }

    $hProc = ::ScalarOpen("$$self{'SD_CMD'} info 2>NUL|");

    while (<$hProc>) {
        chomp;
        if (m/Source\sServer\sDepot:\s(.*)/i) {
            $PreferredAlias = $1;
        } elsif (m/Server\saddress:\s(.*)/i) {
           $Server = $1;
        } elsif (m/Source\sServer\sAddress:(.*)/i) {
           $PreferredServer = $1;
        } elsif (m/Client\sroot:\s(.*)/i) {
            $Root = $1;
        }
    }

    close($hProc);

    #
    # Now, override the preferred alias if 'sd set' gives us a port.
    # (We expect this to always return an SDPORT, but also expect
    # to do the Right Thing if there's an odd case where it doesn't.)
    #
    if ( defined $$self{'SHOWCMDS'} && $$self{'SHOWCMDS'} ) {
        ::status_message("[CWD: $SourceRoot] Running \"$$self{'SD_CMD'} set\"\n");
    }
    $hProc = ::ScalarOpen("$$self{'SD_CMD'} set 2>NUL|");

    while (<$hProc>) {
        chomp;
        if ( m/^SDPORT=(.*:\d+)(\s*\(.*\).*)?/i ) {
            $PreferredServer = $1;
        }
    }

    close($hProc);

    if ( defined $PreferredServer ) {
        $Server = $PreferredServer;
    }

    if ( ! defined $Server ) {
        ::status_message("Server name not returned by $$self{'SD_CMD'} info for $SourceRoot.".
                         "Skipping all files in the depot.");
        return;
    }

    if ( ! defined $$ServerRefs{uc $Server} ) {
        ::warn_message("$Server not found in srcsrv.ini. Skipping all files in depot.");
        return;
    }

    if ( defined $$self{'SHOWCMDS'} && $$self{'SHOWCMDS'} ) {
        ::status_message("[CWD: $SourceRoot] Running \"$$self{'SD_CMD'} have ...\"\n");
    }

    $hProc = ::ScalarOpen("$$self{'SD_CMD'} have ... 2>NUL|");

    FILE: while (<$hProc>) {
        next if     (m/^--/);
        next if     (m/^\s*$/);
        next unless (m/#/);
        chomp;

        if ( $_ =~ m/^(.*)\#(\d*)\s-\s([A-Z]:\\.*)/i ) {
            my $local_file    = $3;
            my $remote_file   = $1;
            my $file_revision = $2;

            $remote_file =~ s/^\/\///gi;

            if (defined $$self{'SD_LABEL'} ) {
                $file_revision = $$self{'SD_LABEL'};
            }

            next if ::ExcludeFile($remote_file, $ExcludesRef);

            # Add the information for this file to the FILE_LOOKUP_TABLE that
            # will be referenced when SSIndex calls out GetFileInfo() function.
            @{$$self{'FILE_LOOKUP_TABLE'}{lc $local_file}} =
                    # First element is a hash of the variables used in this line
                    ( {"$$ServerRefs{uc $Server}" => "$Server"},
                    # Second element is the data for extracting this file
                    # var 1 will be prepended by GetFileInfo()
                    "$$ServerRefs{uc $Server}*". # var 2
                    "$remote_file*".             # var 3
                    "$file_revision");           # var 4
        }
    }

    close($hProc);
    chdir($PrevDir);
}

# -----------------------------------------------------------------------------
# Return ths SRCSRV stream data for a single file.
# -----------------------------------------------------------------------------
sub GetFileInfo {
    my $self        = shift;
    my $file        = shift;
    my $name_in_pdb = shift;

    if ( ! defined $name_in_pdb ) {
        $name_in_pdb = $file;
    }

    # We stored the necessary information when GatherFileInformation() was
    # called so we just need to return that information.
    if ( defined $$self{'FILE_LOOKUP_TABLE'}{lc $file} ) {
        return( ${$$self{'FILE_LOOKUP_TABLE'}{lc $file}}[0],
                "$name_in_pdb*${$$self{'FILE_LOOKUP_TABLE'}{lc $file}}[1]" );
    } else {
        return(undef);
    }
}

# -----------------------------------------------------------------------------
# The long name that should be written the SRCSRV stream to describe
# the source control system being indexed.
# -----------------------------------------------------------------------------
sub LongName {
    return("Source Depot");
}

# -----------------------------------------------------------------------------
# Set the debug level for output.
# -----------------------------------------------------------------------------
sub SetDebugMode {
    my $self = shift;
    $$self{'SD_DEBUGMODE'} = shift;
}

# -----------------------------------------------------------------------------
# Return the SCS specific stream variables.
# -----------------------------------------------------------------------------
sub SourceStreamVariables {
    my $self = shift;
    my @stream;

    # The extraction command varies based on whether or not we're using
    # a label.
    if ( defined $$self{'SD_LABEL'} ) {
        push(@stream, "SD_EXTRACT_CMD=sd.exe -p %fnvar%(%var2%) print ".
                                            "-o %srcsrvtrg% ".
                                            "-q \"//%var3%@%var4%\"");
    } else {
        push(@stream, "SD_EXTRACT_CMD=sd.exe -p %fnvar%(%var2%) print ".
                                            "-o %srcsrvtrg% ".
                                            "-q \"//%var3%#%var4%\"");
    }

    push(@stream, "SD_EXTRACT_TARGET=".
                  "%targ%\\%var2%\\%fnbksl%(%var3%)\\%var4%".
                  "\\%fnfile%(%var1%)");

    push(@stream, "SRCSRVVERCTRL=sd");
    push(@stream, "SRCSRVERRDESC=Connect to server failed");
    push(@stream, "SRCSRVERRVAR=var2");

    return(@stream);
}

# -----------------------------------------------------------------------------
# Loads previously saved file information.
# -----------------------------------------------------------------------------
sub LoadFileInfo {
    my $self = shift;
    my $dir  = shift;

    my $file = "$dir\\sd_files.dat";
    if ( -e  $file ) {
        do $file;
        $$self{'FILE_LOOKUP_TABLE'} = $SD_FILES::FileData1;
    } else {
        ::status_message("No SD information saved in $dir.\n");
    }

    return();
}

# -----------------------------------------------------------------------------
# Saves current file information.
# -----------------------------------------------------------------------------
sub SaveFileInfo {
    my $self = shift;
    my $dir  = shift;
    my $FileLookupRef = $$self{'FILE_LOOKUP_TABLE'};
    my $tempval;

    my $fh;
    if ( defined $FileLookupRef ) {
        $fh = ::ScalarOpen(">$dir\\sd_files.dat");

        if ( $fh ) {
            # Use this so simulate Data::Dumper instead of using
            # Data::Dumper directly since this doesn't require
            # in-memory expansion of the entire hash structure.
            print($fh "\$SD_FILES::FileData1 = {");
            my $key;

            foreach $key (sort keys %{$FileLookupRef} ) {
                $tempval = $key;
                $tempval =~ s/'/\\'/g;

                printf($fh "'%s' => [ {", $tempval);
                foreach (sort keys %{$FileLookupRef->{$key}->[0]}) {
                    printf($fh
                           "'%s' => '%s',",
                           $_,
                           ${$FileLookupRef->{$key}->[0]}{$_});
                }
                $tempval = $FileLookupRef->{$key}->[1];
                $tempval =~ s/'/\\'/g;
                printf($fh "}, '%s'", $tempval);
                printf($fh "],\n                        ");
            }

            print($fh "};");
            close($fh);
        } else {
            ::status_message("Failed to save data to $dir.\n");
        }
    }

    return();
}

# -----------------------------------------------------------------------------
# Simple usage ('-?')
# -----------------------------------------------------------------------------
sub SimpleUsage {
print<<SD_SIMPLE_USAGE;
Source Depot specific settings:

     NAME            SWITCH      ENV. VAR        Default
  -----------------------------------------------------------------------------
  A) sd command     CMD         SD_CMD           sd.exe
  B) label          Label       SD_LABEL         <n/a>
  C) old root       OldRoot     <n/a>            <n/a>
  D) new root       NewRoot     <n/a>            <n/a>
SD_SIMPLE_USAGE
}

# -----------------------------------------------------------------------------
# Verbose usage ('-??')
# -----------------------------------------------------------------------------
sub VerboseUsage {
print<<SD_VERBOSE_USAGE;
(A)  SD Command - The name of the executable to run to issue commands to the
     Source Depot server.  The executable named here must support the same
     options as sd.exe in order for the script to work correctly.

(B)  Label - Use the given text as the file revision label to extract source
     using instead of extracting the source using the numeric file revision.

(C)  Old Root and New Root - Allows the source indexing of symbols that were
     built on another machine.  If these are set, every source path that is
     prefixed with Old Root with have that prefix replaced with the value in New
     Root prior to attempting to resolve the local path and filename to a server
     path and filename. Both Old Root and New Root must be specified to use this
     feature.

(D)  New Root - See Old Root above.
SD_VERBOSE_USAGE
}

1;
__END__
   9) depot map      DepotMap    SD_DEPOTMAP      <n/a>
  10) Source files   FileTable   SD_FILETABLE     <n/A>

(9)  Depot Map and Source Files - See the Source Server documentation for a full
     description of these settings.

(10) Source Files - See Depot Map above.
