import urllib
import os

try:
    os.stat('./download')
except:
    os.mkdir('./download') 


with open('images.xml') as f:
    for line in f:
        filename = line.rstrip('\n')

        # cleanup weird ? at the end of some file.
        if filename[-4:] != '.svg':
          filename = filename[:-1]

        f = urllib.URLopener()
        file_url = 'http://images-staging.transitapp.com/svg/' + filename
        print '\"' + filename + '\"' + ','
        f.retrieve(file_url, 'download/' + filename)

print 'Use previous printed snippet in main.cpp of svgren ios test app.\n Helps validate if your image will work with the library.'
