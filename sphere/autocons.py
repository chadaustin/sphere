# Courtesy of Ben Scott
def ParseConfig(env, command, options):
    "Parses xxx-config style output for compilation directives"

    # Run the command
    where = WhereIs(command)
    if not where:
        print '%s not found in PATH' % command
        sys.exit(-1)
    print "Found " + str(command) + ": " + where
    cmd = where + ' ' + string.join(options)
    params = string.split(os.popen(cmd).read())

    # Parse its output
    for arg in params:
        switch = arg[0:1]
        option = arg[1:2]
        if switch == '-':
            if option == 'I':
                RequireList(env, 'CPPPATH')
                env['CPPPATH'].append(arg[2:])
            elif option == 'L':
                RequireList(env, 'LIBPATH')
                env['LIBPATH'].append(arg[2:])
            elif option == 'l':
                RequireList(env, 'LIBS')
                env['LIBS'].append(arg[2:])
            elif arg[0:11] == '-Wl,-rpath,':
                RequireList(env, 'LINKFLAGS')
                env['LINKFLAGS'].append(arg)
            else:
                RequireList(env, 'CXXFLAGS')
                env['CXXFLAGS'].append(arg)
        else:
            # Must be a static library, add it to the libs
            RequireList(env, 'LIBS')
            env['LIBS'].append(arg)

        RequireList(env, 'LIBS')
        env['LIBS'].extend(['SDL', 'SDLmain'])
        
