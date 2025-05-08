# Welcome to the ProjectAirSimPP test page

Hello!
{# print "The result of (5 * 3 / 16) is ", 5 * 3 / 16, "." #}
{# define pi 3.1415926535 #}{# define radius 3 #}{#print "The area of a circle with radius ", radius, " is ", pi * radius * radius, "." #}
{# include test_projectairsimpp_a.txt #}
{# print "In test.md, is variable b_var defined?  ", defined(b_var)#}
{# print "In test.md, is variable have_included_b defined?  ", defined(have_included_b)#}
{# define RELEASE 1 #}
{# define PROFILE 1 #}
{# print "RELEASE = ", RELEASE, ", PROFILE = ", PROFILE #}
{# print "Is PROFILE defined? ", defined("PROFILE") #}
{# print "Undefining PROFILE" #}
{# undef PROFILE #}
{# print "Is PROFILE defined now? ", defined(PROFILE) #}
{# ifdef PROFILE #}
{# print "Profile build." #}
{# elif RELEASE #}
{# print "Release build." #}
{# else #}
{# print "Debug build." #}
{# endif #}
{# define _ "Hello" #}
{# print "There are ", len(_), " characters in the string \"", _, "\"." #}

{# set filename = "test_projectairsimpp_c.txt" #}
{# print "The contents of ", filename, " is: \"", readfile(filename), '"' #}
{# set c_text readfileline(filename) #}
{# print "The first line of ", filename, " is: \"", c_text, '"' #}
{# print "The minor version number from ", filename, " is: ", field(c_text, ".", 1) #}

{# setlocal local_var "I'm a non-inherited local variable" #}
{# print 'The variable "local_var" is "', local_var, '"' #}
{# include test_projectairsimpp_d.txt #}
{# print 'In test.md, the variable "local_var" is again "', local_var, '"' #}
{# print 'This is a chess knight: \u265e' #}
{# print 'This string contains a ProjectAirSimPP tag: {# log fatal "This example tag should just be output and not be processed." #}' #}

{# print nonsense * 3 #}
{# ifdef true #}
{# print "The ifdef has an illegal variable name" #}
{# endif #}