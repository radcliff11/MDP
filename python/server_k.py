import tornado.ioloop
import tornado.web
import tornado.websocket

import threading
from functools import wraps
from tornado.options import define, options, parse_command_line

import datetime
import json
import zope.event
import random

from algo.constants import *
import algo.sim
from algo.exploration import Exploration
from algo.shortest_path import ShortestPath

import sys

orig_stdout = sys.stdout
f = file('log.txt', 'w')

clients = dict()
started = False
delay_time = 0.1
exp_done = False

define("port", default=8924, help="run on the given port 8924", type=int)

def delay_call(f, *args, **kwargs):
    global delay_time
    t = threading.Timer(delay_time, f, args=args, kwargs=kwargs)
    t.start()

class FuncThread(threading.Thread):
    def __init__(self, target, *args):
        self._target = target
        self._args = args
        threading.Thread.__init__(self)

    def run(self):
        self._target(*self._args)



class WebSocketHandler(tornado.websocket.WebSocketHandler):
    def open(self, *args):
        self.id = self.get_argument("Id")
        self.stream.set_nodelay(True)
        clients[self.id] = {"id": self.id, "object": self}
        tick("INIT")

    def on_message(self, message):
        """
        when we receive some message we want some message handler..
        for this example i will just print message to console
        """
        print ("Client " + str(self.id) + " received a message : " + str(message))
        #for key in clients:
        #    if (clients[key]['id'] != self.id):
        #        clients[key]['object'].write_message(message)

    def on_close(self):
        if self.id in clients:
            del clients[self.id]

class IndexHandler(tornado.web.RequestHandler):
    @tornado.web.asynchronous
    def get(self):
        self.render("display.html")

def start_exploration(percentage, delay):
    global robot
    global started
    global delay_time
    if started:
        return
    started = True

    global f
    if f.closed:
        f = file('log.txt', 'w')
    sys.stdout = f

    robot = algo.sim.Robot()
    delay_time = float(delay)

    robot.get_sensors()
    robot.action(RIGHT)
    robot.get_sensors()
    robot.action(LEFT)
    robot.get_sensors()



    exp = Exploration(int(percentage))

    global sensors
    sensors = robot.get_sensors()

    t1 = FuncThread(exploration, exp)
    t1.start()
    t1.join()

    inform("Exploration started!")

def start_sp_to_goal():
    global robot
    if not exp_done:
        return False
    sp = ShortestPath(robot.explored_map, robot.direction, robot.current, robot.goal)
    sp_list = sp.shortest_path()
    sp_sequence = sp_list['trim_seq']
    sp_sequence.reverse() # will pop from the back
    inform(sp_sequence)
    delay_call(sp_to_goal, sp_sequence)

    inform("ShortestPath started!")

class StartHandler(tornado.web.RequestHandler):
    @tornado.web.asynchronous
    def get(self, percentage, delay):
        self.write("Starting...")
        start_exploration(percentage, delay)
        self.flush()

class StartSpHandler(tornado.web.RequestHandler):
    @tornado.web.asynchronous
    def get(self):
        self.write("Starting...")
        start_sp_to_goal()
        self.flush()



class StopHandler(tornado.web.RequestHandler):
    @tornado.web.asynchronous
    def get(self):
        global started
        inform("Exploration stopped!")
        started = False
        self.flush()


        global f
        sys.stdout = orig_stdout
        f.close()

app = tornado.web.Application([
    (r'/', IndexHandler),
    (r'/ws', WebSocketHandler),
    (r'/start/(.*)/(.*)', StartHandler),
    (r'/start_sp/', StartSpHandler),
    (r'/stop/', StopHandler)
])





def tick(action):
    for key in clients:
        message = dict()
        message['type'] = 'map'
        message['action'] = action
        message['time'] = str(datetime.datetime.utcnow())
        message['map'] = robot.explored_map
        clients[key]['object'].write_message(json.dumps(message))


def inform(string):
    print(string)
    for key in clients:
        message = dict()
        message['type'] = 'info'
        message['time'] = str(datetime.datetime.utcnow())
        message['info'] = string
        clients[key]['object'].write_message(json.dumps(message))

def exploration(exp):
    global started
    if not started:
        return False

    global sensors
    cur = exp.getRealTimeMap(sensors, robot.explored_map)
    if not cur[1]:
        robot.action(cur[0])
        print(robot.current)
        sensors = robot.get_sensors()
        delay_call(exploration, exp)
    else:
        inform("Exploration done!")

        inform(robot.descriptor_one())
        inform(robot.descriptor_two())


        sp = ShortestPath(robot.explored_map, robot.direction, robot.current, robot.start)
        sp_list = sp.shortest_path(-1)
        sp_sequence = sp_list['trim_seq']
        sp_sequence.reverse() # will pop from the back
        inform(sp_sequence)

        # call sp to start
        delay_call(sp_to_start, sp_sequence)

# @delay(delay_time)
def sp_to_start(sequence):
    global started
    global exp_done
    if not started:
        return False
    if len(sequence) == 0:
        exp_done = True
        inform("Gone back to start!")
        return False
    choice = sequence.pop()
    robot.action(choice, -1)
    print(choice, ': ', robot.direction)
    delay_call(sp_to_start, sequence)


# @delay(delay_time)
def sp_to_goal(sequence):
    global started
    if not started:
        return False
    if len(sequence) == 0:
        inform("ShortestPath done!")
        started = False
        return False
    choice = sequence.pop()
    robot.action(choice, 9)
    print(choice, ': ', robot.direction)
    delay_call(sp_to_goal, sequence)

if __name__ == '__main__':
    parse_command_line()
    app.listen(options.port)
    robot = algo.sim.Robot()
    old_subscribers = zope.event.subscribers[:]
    del zope.event.subscribers[:]
    zope.event.subscribers.append(tick)

    print("Listening to http://localhost:" + str(options.port) + "...")
    # tornado.ioloop.IOLoop.instance().start()
    t3 = FuncThread(tornado.ioloop.IOLoop.instance().start)
    t3.start()
    t3.join()
