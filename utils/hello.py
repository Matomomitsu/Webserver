import cgi
import cgitb
import signal
import sys

#while(True):
#   continue

def handler(signum, frame):
    print ('Script took too long - Exiting...')
    sys.exit(1)

signal.signal(signal.SIGALRM, handler)
signal.setitimer(signal.ITIMER_REAL, 5)

form = cgi.FieldStorage()

first_name = form.getvalue('first_name')
last_name  = form.getvalue('last_name')

print ("Content-Type:text/html\r\n\r\n")
print ("<html>")
print ("<head>")
print ("<title>Hello - Second CGI Program</title>")
print ("</head>")
print ("<body>")
if first_name is None or last_name is None:
    print("<h2>Error: missing first name or last name</h2>")
else:
    print ("<h2>Hello " + first_name +  " " + last_name + "</h2>")
print ("</body>")
print ("</html>")
