#!/usr/bin/perl -w
use strict;

###########################################################

die "Usage: $0 <type>\n"
   . "  e.g. $0 txt > doc_functions.txt\n"
   . "  e.g. $0 html > doc_functions.html\n" unless (@ARGV == 1);

die "type must be either txt or html\n" unless ($ARGV[0] eq "txt" || $ARGV[0] eq "html");

my $output_type = $ARGV[0];

sub is_html {
  return $output_type eq "html";
}

sub is_txt {
  return $output_type eq "txt";
}

###########################################################

sub end_of_line {
  if (is_html()) {
    return "<br>\n";
  }
  elsif (is_txt()) {
    return "\n";
  }
}

###########################################################

sub fix_line_endings {
  my ($line) = @_;
  my @lines = split(/\n/, $line);

  if (is_html()) {
    foreach my $line (@lines) {
       $line =~ s/^(\s+)//;
       for (my $i = 0; $i <= length($1); $i++) {
         $line = "&nbsp;$line";
       }
    }
  }

  return join(&end_of_line(), @lines);
}

###########################################################

sub function_to_string {
  my ($func_name, $func_minargs, $desc_text, @func_arg_info) = @_;
  my @func_args = ();
  my @func_arg_types = ();

  for (my $i = 0; $i < @func_arg_info; $i++) {
    if ($i < @func_arg_info / 2) {
      push (@func_args, $func_arg_info[$i]);
    } else {
      push (@func_arg_types, $func_arg_info[$i]);

    }
  }

  my $line = "";
  
  if (&is_html()) { $line .= "<a name=\"$func_name\">"; }
  $line .= "$func_name";
  if (&is_html()) { $line .= "</a>"; }
  
  $line .= "(";

  my $open_bracket = 0;
  for (my $i = 0; $i <= $#func_args; $i++) {

    my $no_comma = 0;

    if ($i >= $func_minargs) {
      if ($open_bracket == 0) {
        if ($i == 0) {
          $line .= "[";
        }
        else {
          $line .= " [, ";
        }
      }
      $open_bracket = 1;
      $no_comma = 1;
    }

    if ($no_comma == 0) {
      if ($i > 0 && $i <= $#func_args) {
        $line .= ", ";
      }
    }

    if (is_html()) {
      $line .= $func_args[$i] . ":<span class='type_$func_arg_types[$i]'>" . $func_arg_types[$i] . "</span>";
    } else {
      $line .= $func_args[$i] . ":" . $func_arg_types[$i];    
    }

  }

  if ($open_bracket == 1) {
    $line .= "]";
  }
  $line .= ")" . &end_of_line();

  if ($desc_text ne "") {
    $line .= &fix_line_endings($desc_text);
    $line .= &end_of_line();
  }

  return $line;
}

###########################################################

sub method_to_string {
  my ($func_name, $func_minargs, $desc_text, @func_args) = @_;
  my $line = function_to_string($func_name, $func_minargs, $desc_text, @func_args);
  my $end_line = &end_of_line();
  my @lines = split(/$end_line/, $line);

  my $prefix = "  ";
  
  if (is_html()) {
    $prefix = "&nbsp;&nbsp;";
  }
  foreach my $line (@lines) {
    $line = $prefix . $line . $end_line;
  }
  
  return join("", @lines) . $end_line;
}

###########################################################

sub ssobject_name_to_jsobject_name {
  my ($method_object) = @_;

  my @ss_names = ("SS_IMAGE", "SS_WINDOWSTYLE", "SS_SURFACE",
                  "SS_FONT", "SS_SOUND", "SS_SPRITESET",
                  "SS_FILE", "SS_LOG", "SS_SOCKET", "SS_ANIMATION",
                  "SS_RAWFILE", "SS_BYTEARRAY", "SS_MAPENGINE");

  my @js_names = ("image", "windowstyle", "surface",
                  "font", "sound", "spriteset",
                  "file", "log", "socket", "animation",
                  "rawfile", "bytearray", "mapengine");

  for (my $i = 0; ($i <= $#js_names && $i <= $#ss_names); $i++) {
    if ($ss_names[$i] eq $method_object) {
      $method_object = $js_names[$i];
      last;
    }
  }
  
  return $method_object;
}

###########################################################

sub ssobject_method_to_jsobject_method {
  my ($method_name) = @_;
  
  my @list = qw(ssSocket ssLog ssSpriteset ssSound ssFont ssWindowStyle ssImage ssSurface ssAnimation ssFile ssByteArray ssRawFile ssMapEngine);

  for (my $i = 0; $i <= $#list; $i++) {
    my $str = $list[$i];

    if ($method_name =~ m/^$str(.*)/) {
      $method_name = $1;
      $method_name = lcfirst($method_name);
      last;
    }
  }

  return $method_name;
}

###########################################################

sub make_docs {
  my $filename = "script.cpp";
  open(IN, $filename) || die("failed to open $filename\n");
  my @lines = <IN>;
  close(IN) || die("failed to close $filename\n");

  my $in_func = 0;
  my $in_method = 0;
  my @args = ();
  my @arg_types = ();

  my $func_name = "";
  my $func_minargs = 0;

  my $method_name = "";
  my $method_object = "";
  my $prev_method_object = "";
  my $method_minargs = 0;

  my $desc_text = "";
  my $in_comment = 0;

  if (is_html()) {
    print "<html>\n";
    print "<head>\n";
    print "\t<title>sphere function list</title>\n";
    print "\t<link rel='stylesheet' href='doc_functions.css' type='text/css' title='Default'>\n";
    print "</head>\n";
    print "<body>\n";
  }

  print "This file is automatically generated, do not edit!" . &end_of_line();

  foreach my $line (@lines) {

    my $no_desc_text = 0;

    # // section: section_name //
    if ($line =~ m/\/\/ section: (.*?) \/\//) {
      print &end_of_line() . "*** $1 ***" . &end_of_line();
    }

    if ($in_comment == 0 && $in_func == 0 && $in_method == 0) {
      # /**
      if ($line =~ m/(\s*)\/\*\*/) {
        $in_comment = 1;
        $no_desc_text = 1;
      }
    }

    if ($in_comment == 1) {
      # */
      if ($line =~ m/(\s*)\*\//) {
        $no_desc_text = 1;
        $in_comment = 0;
      }
    }

    if (!$no_desc_text && $in_comment == 1) {
      my $temp_line = $line;

      # @see name
      if ($temp_line =~ m/\@see (.*)/) {
        my $r = $1;
        $temp_line =~ s/\@see $r/\@see <a href=\"#$r\">$r<\/a>/;
      }

      $desc_text .= $temp_line;

    }

    if ($in_func == 0) {

      # begin_func(func_name, minargs)
      if ($line =~ m/begin_func\((.*?), (.*?)\)/) {
        $in_func = 1;
        $func_name = $1;
        $func_minargs = $2;
      }

    }

    if ($in_method == 0) {

      # begin_method(Object, method_name, minargs)
      if ($line =~ m/begin_method\((.*?), (.*?), (.*?)\)/) {
        $in_method = 1;
        $method_object = $1;
        $method_name = $2;
        $method_minargs = $3;
      }

    }

    if ($in_func == 1 || $in_method == 1) {

      # arg_str(name)
      if ($line =~ m/arg_str\((.*?)\)/) {
        push (@arg_types, "string");
        push (@args, "$1");
      }
      # name = argStr(cx, val)
      if ($line =~ m/\s*(.*?) = argStr\((.*?), (.*?)\)/) {
        push (@arg_types, "string");
        push (@args, "$1");
      }

      # arg_int(name)
      if ($line =~ m/arg_int\((.*?)\)/) {
        push (@arg_types, "int");
        push (@args, "$1");
      }
      # name = argInt(cx, val)
      if ($line =~ m/\s*(.*?) = argInt\((.*?), (.*?)\)/) {
        push (@arg_types, "int");
        push (@args, "$1");
      }

      # arg_bool(name)
      if ($line =~ m/arg_bool\((.*?)\)/) {
        push(@arg_types, "boolean");
        push(@args, "$1");
      }
      # name = argBool(cx, val)
      if ($line =~ m/^\s*(.*?) = argBool\((.*?), (.*?)\)/) {
        push (@arg_types, "boolean");
        push (@args, "$1");
      }

      # arg_double(name)
      if ($line =~ m/arg_double\((.*?)\)/) {
        push (@arg_types, "double");
        push (@args, "$1");
      }
      # name = argDouble(cx, val)
      if ($line =~ m/^\s*(.*?) = argDouble\((.*?), (.*?)\)/) {
        push (@arg_types, "double");
        push (@args, "$1");
      }

      # arg_colormatrix(name)
      if ($line =~ m/arg_colormatrix\((.*?)\)/) {
        push (@arg_types, "colormatrix");
        push (@args, "$1");
      }

      # arg_color(name)
      if ($line =~ m/arg_color\((.*?)\)/) {
        push (@arg_types, "color");
        push (@args, "$1");
      }

      # arg_image(name)
      if ($line =~ m/arg_image\((.*?)\)/) {
        push (@arg_types, "image");
        push (@args, "$1");
      }

      # arg_array(name)
      if ($line =~ m/arg_array\((.*?)\)/) {
        push (@arg_types, "array");
        push (@args, "$1");
      }

      # arg_object(name)
      if ($line =~ m/arg_object\((.*?)\)/) {
        push (@arg_types, "object");
        push (@args, "$1");
      }

      # arg_byte_array(name)
      if ($line =~ m/arg_byte_array\((.*?)\)/) {
        push (@arg_types, "byte_array");
        push (@args, "$1");
      }

      # arg_spriteset(name)
      if ($line =~ m/arg_spriteset\((.*?)\)/) {
        push (@arg_types, "spriteset");
        push (@args, "$1");
      }

      # arg_font(name)
      if ($line =~ m/arg_font\((.*?)\)/) {
        push (@arg_types, "font");
        push (@args, "$1");
      }

      # end_func()
      if ($in_func == 1 && $line =~ m/end_func\(\)/) {

        unless ($func_name eq "name") {
          print &end_of_line();          
          print function_to_string($func_name, $func_minargs, $desc_text, @args, @arg_types);
        }

        $func_name = "";
        $func_minargs = 0;
        @args = ();
        @arg_types = ();
        $in_func = 0;
        $desc_text = "";

      }

      # end_method()
      if ($in_method == 1 && $line =~ m/end_method\(\)/) {
        unless ($method_object eq "Object") {

          my $name = &ssobject_name_to_jsobject_name($method_object);
          my $prefix = is_html() ? "&nbsp;&nbsp;" : "  ";

          if ($prev_method_object ne $method_object) {
            print &end_of_line();
            print uc(&ssobject_name_to_jsobject_name($method_object)) . &end_of_line();
            if ($method_object eq "color") {
              print "$prefix$name.red" . &end_of_line() . &end_of_line();
              print "$prefix$name.green" . &end_of_line() . &end_of_line();
              print "$prefix$name.blue" . &end_of_line() . &end_of_line();
              print "$prefix$name.alpha" . &end_of_line() . &end_of_line();
            }

            if ($method_object eq "SS_IMAGE" || $method_object eq "SS_SURFACE" || $method_object eq "SS_ANIMATION") {
              print "$prefix$name.width" . &end_of_line() . &end_of_line();
              print "$prefix$name.height" . &end_of_line() . &end_of_line();
            }

            if ($method_object eq "SS_BYTEARRAY") {
              print "$prefix$name" . "[index]" . &end_of_line() . &end_of_line();
              print "$prefix$name.length" . &end_of_line() . &end_of_line();
            }
          }

          if ($method_object eq "SS_FILE") {
            if ($method_name eq "ssFileWrite" || $method_name eq "ssFileRead") {
              push (@args, "default_value");
              push (@arg_types, "void");
            }
          }

          print method_to_string(&ssobject_name_to_jsobject_name($method_object) . "." . &ssobject_method_to_jsobject_method($method_name), $method_minargs, $desc_text, @args, @arg_types);
        }

        $method_name = "";
        $prev_method_object = $method_object;
        $method_object = "";
        $method_minargs = 0;
        @args = ();
        @arg_types = ();
        $in_method = 0;
        $desc_text = "";

      }
    }

  }

  if (is_html()) {
    print "</body></html>";
  }
}

###########################################################

&make_docs();
