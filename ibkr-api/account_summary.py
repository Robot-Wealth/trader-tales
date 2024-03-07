from threading import Thread, Event
import time
from typing import Any
from ibapi.wrapper import EWrapper
from ibapi.client import EClient
from ibapi.common import *
from ibapi.account_summary_tags import AccountSummaryTags


class ibapp(EClient, EWrapper):
    def __init__(self):
        EClient.__init__(self, self)
        self.done = Event()  # use threading.Event to signal between threads
        self.connection_ready = Event()  # to signal the connection has been established

    # override Ewrapper.error
    def error(
        self, reqId: TickerId, errorCode: int, errorString: str, contract: Any = None
    ):
        print("Error: ", reqId, " ", errorCode, " ", errorString)
        if errorCode == 502:  # not connected
            # set self.done (a threading.Event) to True
            self.done.set()

    # override Ewrapper.accountSummary - method for receiving account summary
    def accountSummary(
        self, reqId: int, account: str, tag: str, value: str, currency: str
    ):
        # just print the account information to screen
        print(
            "AccountSummary. ReqId:",
            reqId,
            "Account:",
            account,
            "Tag: ",
            tag,
            "Value:",
            value,
            "Currency:",
            currency,
        )

    # override Ewrapper.accountSummaryEnd - notifies when account summary information has been received
    def accountSummaryEnd(self, reqId: int):
        # print to screen
        print("AccountSummaryEnd. ReqId:", reqId)
        # set self.done (a threading.Event) to True
        self.done.set()

    # override Ewrapper.nextValidID - used to signal that the connection between application and TWS is complete
    # returns the next valid orderID (for any future transactions)
    # if we send messages before the connection has been established, they can be low
    # so wait for this method to be called
    def nextValidId(self, orderId: int):
        print(f"Connection ready, next valid order ID: {orderId}")
        self.connection_ready.set()  # signal that the connection is ready


# define our event loop - this will run in its own thread
def run_loop(app):
    app.run()


# instantiate an ibapp
app = ibapp()

# connect
app.connect("127.0.0.1", 7496, clientId=0)  # clientID identifies our application

# start the application's event loop in a thread
api_thread = Thread(target=run_loop, args=(app,), daemon=True)
api_thread.start()

# wait until the Ewrapper.nextValidId callback is triggered, indicating a successful connection
app.connection_ready.wait()

# request account summary
print("Requesting account summary")
app.reqAccountSummary(0, "All", AccountSummaryTags.AllTags)

# wait for the account summary to finish (ie block until app.done - a threading.Event - becomes true)
app.done.wait()

# disconnect
app.disconnect()
