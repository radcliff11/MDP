import tornado.httpserver
import tornado.ioloop
import tornado.web
import tornado.websocket
import tornado.gen
import multiprocessing

import threading
from functools import wraps
from tornado.options import define, options, parse_command_line

import datetime
import json
import zope.event
import random

import time
import bluetooth
from bluetooth import *
import gevent
import serial
from gevent import socket
from gevent.event import Event

from collections import deque


from algo.constants import *
import algo.real
from algo.exploration import Exploration
from algo.shortest_path import ShortestPath

from gevent import monkey
monkey.patch_all()

import sys

orig_stdout = sys.stdout
f = file('log.txt', 'w')


clients = dict()
started = False
delay_time = 0
evt = Event()
sensors = []
android_ok = False
exp_done = False
io_loop = False
exploration_started = False
sp_to_goal_started = False


#global btsock
btsock = None


define("port", default=8888, help="run on the given port", type=int)

def delay_call(f, *args, **kwargs):
    evt.wait()
    # ignore delay_time, don't spawn new thread
    f(*args, **kwargs)

    # global delay_time
    # t = threading.Timer(delay_time, f, args=args, kwargs=kwargs)
    # t.start()

def real_delay_call(f, delay_time, *args, **kwargs):
    t = threading.Timer(delay_time, f, args=args, kwargs=kwargs)
    t.start()

class FuncThread(threading.Thread):
    def __init__(self, target, *args):
        self.__target = target
        self.__args = args
        threading.Thread.__init__(self)

    def run(self):
        self.__target(*self.__args)


class WebSocketHandler(tornado.websocket.WebSocketHandler):
    def open(self, *args):
        self.id = self.get_argument("Id")
        self.stream.set_nodelay(True)
        clients[self.id] = {"id": self.id, "object": self}
        tick("INIT")

    def on_message(self, message):
        print ("[Tornado | " + time.ctime(time.time()) + "] WSHandler > Client " + str(self.id) + " received a message : " + str(message))

    def on_close(self):
        if self.id in clients:
            del clients[self.id]

class IndexHandler(tornado.web.RequestHandler):
    @tornado.web.asynchronous
    def get(self):
        self.render("display.html")

class StartHandler(tornado.web.RequestHandler):
    @tornado.web.asynchronous
    def get(self, percentage, delay):
        self.write("Starting...")

        t = FuncThread(start_exploration, 100, 0.0)
        t.start()

        self.flush()

class StartSpHandler(tornado.web.RequestHandler):
    @tornado.web.asynchronous
    def get(self):
        self.write("Starting Sp...")

        t = FuncThread(start_sp_to_goal)
        t.start()

        self.flush()


class StopHandler(tornado.web.RequestHandler):
    @tornado.web.asynchronous
    def get(self):
        global started
        inform("Halted!")
        started = False


        global f
        sys.stdout = orig_stdout
        f.close()

        self.flush()

app = tornado.web.Application([
    (r'/', IndexHandler),
    (r'/ws', WebSocketHandler),
    (r'/start/(.*)/(.*)', StartHandler),
    (r'/start_sp/', StartSpHandler),
    (r'/stop/', StopHandler)
])


def tick(action):
    #if android_ok:
    #    btWrite(robot.msg_for_android())
    for key in clients:
        message = dict()
        message['type'] = 'map'
        message['action'] = action
        message['time'] = str(datetime.datetime.utcnow())
        message['map'] = robot.explored_map
        clients[key]['object'].write_message(json.dumps(message))

def inform(string):
    print("[Tornado | %s] inform > %s " %(time.ctime(time.time()), string))

    for key in clients:
        message = dict()
        message['type'] = 'info'
        message['time'] = str(datetime.datetime.utcnow())
        message['info'] = string
        clients[key]['object'].write_message(json.dumps(message))

def do_alignment(actions):
    global started
    #global doing_sp
    if not started or len(actions) <= 0:
        return False
    choice = actions[0]
    actions = actions[1:]
    send_cmd(choice)
    if choice == RIGHT or choice == LEFT:
        #if doing_sp:
        #    actions.push(LEFT)
        robot.action(choice)
    #elif choice == LEFT:
    #    robot.action(choice)

    gevent.joinall([
        gevent.spawn(delay_call, do_alignment, actions)
    ])

#####################
###    exploration
#####################

def start_exploration(percentage, delay):
    global robot
    global started
    global delay_time
    global exploration_started


    if exploration_started:
        return
    exploration_started = True

    if started:
        return
    global f
    if f.closed:
        f = file('log.txt', 'w')
    sys.stdout = f


    inform("Exploration started: Alignment!")
    robot = algo.real.Robot()
    delay_time = float(delay)

    send_cmd(FD_ALIGN) # W
    evt.wait()
    send_cmd(LD_ALIGN) # Q
    evt.wait()
    send_cmd(FD_ALIGN) # W
    evt.wait()
    send_cmd(RIGHT) # D
    evt.wait()
    send_cmd(LA_ALIGN) # L
    evt.wait()

    started = True
    send_cmd(REQ_SENSOR) # E
    evt.wait()

    send_cmd(RIGHT)
    robot.action(RIGHT)
    evt.wait()

    send_cmd(REQ_SENSOR) # E
    evt.wait()

    send_cmd(LEFT)
    robot.action(LEFT)
    evt.wait()

    started = False
    send_cmd(LD_ALIGN) # Q
    evt.wait()
    send_cmd(LA_ALIGN) # L
    evt.wait()
    started = True


    ### TESTING
    # for i in range(robot.MAX_ROW):
    #     for j in range(robot.MAX_COL):
    #         robot.explored_map[i][j] = 1
    # for i in range(7):
    #     robot.explored_map[4][i] = 2
    # for i in range(5):
    #     robot.explored_map[i][6] = 2


    # # doing_sp = True
    # sp = ShortestPath(robot.explored_map, robot.direction, robot.current, robot.goal)
    # sp_list = sp.shortest_path(-1)
    # sp_sequence = sp_list['trim_seq']
    # sp_sequence.reverse()
    # inform(sp_sequence)

    # gevent.joinall([
    #     gevent.spawn(sp_to_goal, sp_sequence)
    # ])
    ### END TESTING


    exp = Exploration(int(percentage))

    inform("Exploration started, for real!")
    t1 = FuncThread(exploration, exp)
    t1.start()
    t1.join()

def exploration(exp):
    global started
    if not started:
        return False

    evt.wait()
    do_alignment(robot.alignment())

    global sensors
    cur = exp.getRealTimeMap(sensors, robot.explored_map)
    if cur[1]:
        done_exploration()
        return False

    if cur[0]:
        if robot.try_left:
            evt.wait()
            do_alignment(robot.alignment())

        robot.action(cur[0])
        send_cmd(cur[0])

    print("[Tornado] exploration > %s" %(robot.current))

    gevent.joinall([
        gevent.spawn(delay_call, exploration, exp)
    ])

def done_exploration():
    global started
    inform("Exploration done!")

    started = False

    inform(robot.descriptor_one())
    inform(robot.descriptor_two())
    # inform(robot.msg_for_android())

    sp = ShortestPath(robot.explored_map, robot.direction, robot.current, robot.start)
    sp_list = sp.shortest_path(-1)
    sp_sequence = sp_list['trim_seq']
    sp_sequence.reverse()
    inform(sp_sequence)

    # call sp to start
    gevent.joinall([
        gevent.spawn(sp_to_start, sp_sequence)
    ])

#####################
###    sp_to_start
#####################

def sp_to_start(sequence):
    global started
    #if not started:
    #    return False
    if len(sequence) == 0:
        done_sp_to_start()
        return False

    evt.wait()
    do_alignment(robot.alignment())

    choice = sequence.pop()
    robot.action(choice, -1)
    send_cmd(choice)

    print("[Tornado | %s] sp_to_start > %s : %s" %(time.ctime(time.time()), choice, robot.direction))
    gevent.joinall([
        gevent.spawn(sp_to_start, sequence)
    ])

def done_sp_to_start():
    global exp_done
    inform("Gone back to start: Alignment!")

    evt.wait()
    send_cmd("W")
    evt.wait()

    # Calibrate first!
    if robot.direction == NORTH:
        robot.action(LEFT)
        send_cmd(LEFT)
        evt.wait()
    elif robot.direction == SOUTH:
        robot.action(RIGHT)
        send_cmd(RIGHT)
        evt.wait()
    elif robot.direction == EAST:
        robot.action(LEFT)
        send_cmd(LEFT)
        evt.wait()
        robot.action(LEFT)
        send_cmd(LEFT)
        evt.wait()

    send_cmd(FD_ALIGN) # W
    evt.wait()

    send_cmd(LD_ALIGN) # Q
    evt.wait()

    robot.action(RIGHT)
    send_cmd(RIGHT) # D
    evt.wait()

    send_cmd(LA_ALIGN) # L
    evt.wait()


    inform(robot.descriptor_one())
    inform(robot.descriptor_two())


    exp_done = True
    global exploration_started
    exploration_started = False
    inform("Gone back to start, for real!")

#####################
###    sp_to_goal
#####################

def start_sp_to_goal():
    global robot
    ## Don't "started" so the map won't update
    #global started
    global sp_to_goal_started
    if sp_to_goal_started:
        return
    sp_to_goal_started = True

    # # TESTING
    # exp_done = True
    # started = True
    # for i in range(robot.MAX_ROW):
    #     for j in range(robot.MAX_COL):
    #         robot.explored_map[i][j] = 1
    # for i in range(6):
    #     robot.explored_map[13][i] = 2
    # robot.explored_map[0][8] = 2
    # robot.direction = NORTH
    # send_cmd(REQ_SENSOR)
    # # END TESTING

    if not exp_done:
        return False
    #started = True
    inform("ShortestPath started!")

    sp = ShortestPath(robot.explored_map, robot.direction, robot.current, robot.goal)
    sp_list = sp.shortest_path()
    sp_sequence = sp_list['trim_seq']
    sp_sequence.reverse()
    inform(sp_sequence)
    gevent.joinall([
        gevent.spawn(sp_to_goal, sp_sequence)
    ])

def sp_to_goal(sequence):

    if len(sequence) == 0:
        done_sp_to_goal()
        return False

    evt.wait()
    # DON'T DO ALIGNMENT WHILE DOING FASTEST PATH RACE
    # do_alignment(robot.alignment())

    choice = sequence.pop()
    robot.action(choice, 9)
    send_cmd(choice)

    print("[Tornado | %s] sp_to_goal > %s : %s" %(time.ctime(time.time()), choice, robot.direction))
    gevent.joinall([
        gevent.spawn(sp_to_goal, sequence)
    ])

def done_sp_to_goal():
    #global started
    inform("ShortestPath done: Alignment!")
    evt.wait()
    send_cmd("W")
    evt.wait()
    inform("ShortestPath done, for real!")
    #started = False
    global sp_to_goal_started
    sp_to_goal_started = False

    global f
    sys.stdout = orig_stdout
    f.close()


#####################
###    BLUETOOTH
#####################

def btComm():
    # btaddr = "00:E3:B2:A1:8F:65" #note3
    btaddr = "08:60:6E:A5:89:46" #nexus
    uuid = "00001101-0000-1000-8000-00805f9b34fb"
    global android_ok
    while not android_ok:
        service_matches = bluetooth.find_service(uuid=uuid, address= btaddr)
        if len(service_matches) == 0:
            print("[Bluetooth | %s] Couldn't find service" %(time.ctime(time.time())))
            continue
        first_match = service_matches[0]
        port = first_match["port"]
        name = first_match["name"]
        host = first_match["host"]
        #port = 4
        btsock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
        btsock.connect((host, port))
        print ("[Bluetooth | %s] btComm > Connected BT" %(time.ctime(time.time())))
        android_ok = True
    return btsock

def btCommListen():
    server_sock = BluetoothSocket(RFCOMM)
    # server_sock.bind(("",PORT_ANY))
    server_sock.bind(("",4))
    server_sock.listen(1)

    port = server_sock.getsockname()[1]
    uuid = "00001101-0000-1000-8000-00805f9b34fb"
    advertise_service( server_sock, "RPIServer",
                   service_id = uuid,
                   service_classes = [ uuid, SERIAL_PORT_CLASS ],
                   profiles = [ SERIAL_PORT_PROFILE ],
                   )

    print("[Bluetooth | %s] Waiting for connection on RFCOMM channel %d" %(time.ctime(time.time()), port))
    client_sock, client_info = server_sock.accept()
    print("[Bluetooth | %s] Accepted connection from %s" %(time.ctime(time.time()), client_info))
    return client_sock

def btWrite(threadName, delay):
    global btsock
    if not btsock:
        btsock = btCommListen()
        btsock.setblocking(0)
    else:
        if len(btq) > 0:
            msg = btq.popleft()
            btsock.send(msg)
            print ("[Android | %s] btWrite > %s" %(time.ctime(time.time()), msg))
    real_delay_call(btWrite, delay, threadName, delay)

def btRead(threadName, delay):
    global started
    try:
        msg = btsock.recv(1024)
        print ("[Android | %s] btRead > %s" %(time.ctime(time.time()), msg))
        if msg == "beginExplore":
            t = FuncThread(start_exploration, 100, 0.0)
            t.start()
        elif msg == "beginFastest":
            t = FuncThread(start_sp_to_goal)
            t.start()
        elif msg == "beginManual":
            global sp_to_goal_started
            global exploration_started
            inform("Halted!")
            robot.direction = NORTH
            started = False
            sp_to_goal_started = False
            exploration_started = False

            global f
            sys.stdout = orig_stdout
            f.close()
        # elif msg == "f":
        #     orig_started = started
        #     started = True
        #     send_cmd(FORWARD)
        #     robot.action(FORWARD)
        #     evt.wait()
        #     started = orig_started()
        # elif msg == "tl":
        #     orig_started = started
        #     started = True
        #     send_cmd(LEFT)
        #     robot.action(LEFT)
        #     evt.wait()
        #     started = orig_started()
        # elif msg == "tr":
        #     orig_started = started
        #     started = True
        #     send_cmd(RIGHT)
        #     robot.action(RIGHT)
        #     evt.wait()
        #     started = orig_started

    except:
        None
    real_delay_call(btRead, delay, threadName, delay)

#####################
###    ARDUINO
#####################

def setSerComm():
    sersock = serial.Serial('/dev/ttyACM0', 115200) # Establish the connection wi$
    return sersock

def serRead(threadName, delay):
    if serial.inWaiting() > 0:
        msg = serial.readline()
        print ("[Arduino | %s] serRead > %s" %(time.ctime(time.time()), msg))
        parse_msg(msg)
    real_delay_call(serRead, delay, threadName, delay)

def serWrite(threadName, delay):
    if len(serialq) > 0:
        msg = serialq.popleft()
        serial.write(msg)
        print ("[Arduino | %s] serWrite > %s" %(time.ctime(time.time()), msg))

    real_delay_call(serWrite, delay, threadName, delay)

def send_cmd(cmd):
    serialq.append(cmd)
    evt.clear()

def parse_msg(msg):
    global started
    print("[Arduino | %s] parse_msg > %s"%(time.ctime(time.time()), msg))

    if (msg == "K" or len(msg) < 5 or msg.startswith("Error")):
        # alignment acknowledgemnet
        None
    elif started:
        sensorString = msg
        global sensors
        sensors = robot.parse_sensors(sensorString)
        robot.update_map()
    evt.set()
    return


if __name__ == '__main__':
    parse_command_line()
    app.listen(options.port)
    robot = algo.real.Robot()
    old_subscribers = zope.event.subscribers[:]
    del zope.event.subscribers[:]
    zope.event.subscribers.append(tick)
    io_loop = tornado.ioloop.IOLoop.instance()


    btsock = btCommListen()
    btsock.setblocking(0)
    serial = setSerComm()

    btq = deque([])
    serialq = deque([])
    print("[Tornado | " + time.ctime(time.time()) + "] > Listening to http://localhost:" + str(options.port) + "...")

    started = False
    t1 = FuncThread(serWrite, "Thread 1-serWrite", 0.1)
    t2 = FuncThread(serRead, "Thread 2-serRead", 0.1)
    t3 = FuncThread(io_loop.start)
    t4 = FuncThread(btWrite, "Thread 4-btWrite", 0.1)
    t5 = FuncThread(btRead, "Thread 5-btRead", 0.1)

    t1.start()
    t2.start()
    t4.start()
    t5.start()

    t3.start()

    #t1.join()
    #t2.join()
    t3.join()
    #t4.join()
    #t5.join()
