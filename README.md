# Turtle drawing

## Wifi Setup

When it first starts the turtle will create a wifi access point called 'Turtle'. Connect to this on a computer or phone and point a browser to '192.168.4.1'. A configuration screen will be displayed which will let you enter the name and pass key for your local wifi. After restarting the turtle will try to connect using the saved wifi details and will start to broadcast it's network address.

If the turtle has previously connected to wifi somewhere else it will try those details first and, if it can't connect, it will create it's own wifi access point so that wifi details for its new location can be entered.

You can use the Python code below to detect the turtle's wifi messages and note down it's network address. The address will be 4 numbers separated by stops e.g. 192.168.1.78. If you don't see any messages please check that your computer's security software lets through `UDP Broadcast` messages.

```python
import socket

# Define the UDP IP address and port to listen on
UDP_IP = ""
UDP_PORT = 4444

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening for UDP packets on {UDP_IP}:{UDP_PORT}")

while True:
    # Receive data from the socket
    data, addr = sock.recvfrom(1024)
    print(f"received packet from {addr}: {data.decode('utf-8')}")
```

## Controlling the Turtle

You can control the turtle by sending web messages via wifi. You can do this with a web browser but it is more powerful to use code such as Python. There is more information about the Python code later.

The format of the web messages looks like this.

```text
http://<turtle network address>/cmd.cgi?<instruction>=<parameter>
```

In place of `<turtle network address>` put the address you noted down earlier.
Here is what you can use for the `<instruction>` and `<parameter>` parts:

- `move=1.0` The turtle will move in a straight line and the parameter says how many times to rotate the wheels. The wheels can move partial turns (for example 0.37) and backwards (for example -2.5). Try some different positive and negative numbers to see what happens.
- `turn=1.0` The turtle will rotate on the spot. The parameter says how many time the wheels rotate during the turn. Just like `move` you can do partical turns and go clockwise or anticlockwise.
- `pen=45` Sets the position of servo which raises or lowers the pen. The parameter depends upon how the pen has been put into the turtle. Usually a negative number will lift the pen off the paper and and number more than 45 will let the pen rest on the paper.

## Working with Python

Here are some hints about how to control the turtle with Python.

### Web requests

Python has a module called `requests` to help work with web messages. Before you can use it you need to install it with this command:

```sh
pip install requests
```

Try this little bit of code to see how the module works.

```python
import requests

webMessage='https://bbc.co.uk'

response=requests.get(webMessage)
print(response.reason)
print(response.text)
```

To control the turtle you will need to replace `https://bbc.co.uk` with the appropriate web message. Try it and see what happens. You can keep doing this by hand but you will be able to do more if you create the message with code.

### Template strings

In Python, template strings can let you make a web message with code.

Try this example and see what happens. Make some changes and see if you can work out what is going on.

```python
someVariable = 'hello'
anotherVariable = 'world'

u = f'This messages includes parts from variables. {someVariable} {anotherVariable}'

print(u)
```

### Puting it all together

Here are some hints.

- It may help to use three variables. They can be for the turtle network address, the instruction and parameter
- A template string using these variables will look a bit like

    `f'http://{ ... }/cmd.cgi?{ ... }={ ... }'`

    You will have to work out what to put in place of `...`
- It may help to define some functions to make it easier to use the turtle. Some examples might start with:

    ```python
    def moveForward(distance):
        ...
        ...

    def penUp():
        ...
        ...

- Can you practice to find out the amount to move the turtle to move precisely 10cm?

- Can you practice to fine out the amount to turn the turtle to get it to face back to where it started (360 degrees)?
- The turtle only understands turns of its wheels. For us it would be more useful to use `cm` for moves and `degrees` for turns. How can you use some mathematics in Python to convert our distances and angles into the right number of wheel rotations?
- Can you work out how to draw a square or triangle with the turtle?

### Imaginary turtles

When you are trying to make something more complex you could easily waste a lot of paper and ink. You can use an imaginary turtle in Python to practice. Python has a module called `turtle` that replaces the physical turtle with an arrow on the screen. Try this code and experiment to see what you can make.

```python
from turtle import *

size=100

for side in range(4):
  forward(size)
  right(90)
```
