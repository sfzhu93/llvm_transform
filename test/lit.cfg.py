import lit.formats

config.name = 'PeepholeTest'
config.test_format = lit.formats.ShTest(True)

config.suffixes = ['.c']  # Specify which file extensions to include in the tests

# Paths to the tools you're using
