#! /usr/bin/python
import serial
import os
import struct
import time

import Queue as queue
import threading

import pensel_utils as pu


class PenselError(RuntimeError):
    pass


class Pensel(object):
    def __init__(self, serialport, baudrate, verbose=False):
        self.serialport = serialport
        self.baudrate = baudrate
        self.verbose = verbose
        self.MAGIC_NUM_0 = 0xBE
        self.MAGIC_NUM_1 = 0xEF
        self.MAGIC_REPLY_0_EXPECTED = 0xDE
        self.MAGIC_REPLY_1_EXPECTED = 0xAD

        # open serial port
        if self.verbose:
            print("Opening serial port...")
        self.serial = serial.Serial(self.serialport, self.baudrate, timeout=0.5)
        if self.verbose:
            print("Opened!")

        # start listening for reports from Pensel
        self.start_listener()
        self._clear_serial = False

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.verbose:
            print("Cleaning up...")
        self.close()

    def serial_write(self, values_to_write):
        """
        Writes `values_to_write` to the serial port.
        """
        if self.verbose:
            print("Writting 0x{:x} to serial port...".format(values_to_write))
        if type(values_to_write) is not list:
            self.serial.write(bytearray([values_to_write]))
        else:
            self.serial.write(bytearray(values_to_write))

    def serial_read(self, num_bytes):
        """
        reads `num_bytes` from the serial port.
        """
        if self.verbose:
            print("Starting to read serial")
        out = self.serial.read(num_bytes)
        if len(out) != num_bytes:
            print("WARNING: Didn't get the expected number of bytes")
            print("    Received {}, expected {}. Serial port dead?".format(len(out), num_bytes))

        out_list = [ord(c) for c in out]
        if self.verbose:
            print("Read in `{}`".format(out_list))

        return out_list

    def serial_clear(self):
        """ Clears the serial buffer of anything received. """
        self.serial.reset_input_buffer()

    def num_bytes_available(self):
        """
        Returns the number of bytes in the input buffer.
        """
        return self.serial.in_waiting

    def start_listener(self):
        self.thread_run = True
        self.queue = queue.Queue()
        self.thread = threading.Thread(target=self.listener)  # , args=(self,)
        self.thread.start()  # start it off

    def packets_available(self):
        return not self.queue.empty()

    def listener(self):
        """ The threaded listener that looks for packets from Pensel. """
        while self.thread_run is True:
            if self.num_bytes_available() != 0 and self.check_for_start():
                report, retval, payload = self.receive_packet()
                if report >= 0:
                    self.queue.put((report, retval, payload))

    def get_packet(self):
        if self.thread.is_alive() is False:
            raise RuntimeError("Thread is dead!!")

        if not self.queue.empty():
            return self.queue.get_nowait()

        return None

    def get_packet_withreportID(self, reportID):
        """
        Returns the first found packet with the given reportID while keeping
        the rest of the packets on the queue in the correct order.
        """
        # check if we've got a correct packet in the queue
        incorrect_packets = []
        correct_pkt = None
        while True:
            pkt = self.get_packet()
            if pkt:
                report, retval, payload = pkt
                # check if it's the correct report
                if reportID == report:
                    correct_pkt = pkt
                    break
                else:
                    incorrect_packets.append(pkt)
            else:
                # no more packets to receive
                break

        # put back incorrect packets onto the queue
        for pkt in incorrect_packets[::-1]:
            self.queue.put(pkt)

        return correct_pkt

    def clear_queue(self):
        while True:
            try:
                self.queue.get_nowait()
            except queue.Empty:
                if self.verbose:
                    print("Queue cleared!")
                break

    def send_report(self, report_ID, payload=None):
        """
        Sends a report to the pensel and reads back the result
        """
        if report_ID < 0 or report_ID > 0x255:
            raise ValueError("Report ID is out of the valid range!")

        self.serial_write(self.MAGIC_NUM_0)
        self.serial_write(self.MAGIC_NUM_1)
        self.serial_write(report_ID)
        if payload is None:
            self.serial_write(0)
        else:
            self.serial_write(len(payload))
            for b in payload:
                if b < 0 or b > 255:
                    raise ValueError("Value in payload out of valid range!")
                self.serial_write(b)

        # Try to get the response
        TIMEOUT = 2  # seconds
        retval = None
        payload = None
        start_time = time.time()
        while True:
            pkt = self.get_packet_withreportID(report_ID)
            if pkt:
                report, retval, payload = pkt
            else:
                time.sleep(0.01)

            # check for timeout
            if time.time() - start_time > TIMEOUT:
                break

        return retval, payload

    def check_for_start(self):
        """
        Checks for the start of a reply from Pensel.
        """
        data = self.serial_read(2)
        if len(data) == 2:
            if data[0] == self.MAGIC_REPLY_0_EXPECTED:
                if data[1] == self.MAGIC_REPLY_1_EXPECTED:
                    # Both bytes are what we expect!
                    if self.verbose:
                        print("Detected start!")
                    return True
                else:
                    # first byte correct, but second isn't. False activation
                    pass
            elif data[1] == self.MAGIC_REPLY_0_EXPECTED:
                # maybe we were just off by 1
                data = self.serial_read(1)
                if len(data) == 1 and data[0] == self.MAGIC_REPLY_1_EXPECTED:
                    # We've got a start!
                    if self.verbose:
                        print("Detected start!")
                    return True

        # default, no start :(
        if self.verbose:
            print("Failed to detect start...")
        return False

    def receive_packet(self):
        """
        Receives a packet, whether from a report reply or an input report,
        from Pensel. Doesn't check for start of packet. That's `check_for_start`
        """
        report = self.serial_read(1)
        if len(report) != 1:
            print("ERROR: Didn't read back a report!")
            report = -1
        else:
            report = report[0]
        retval = self.serial_read(1)
        if len(retval) != 1:
            print("ERROR: Didn't read back a return value!")
            retval = -1
        else:
            retval = retval[0]

        return_payload_len = self.serial_read(1)
        if len(return_payload_len) != 1:
            print("ERROR: Didn't read back a return payload length!")
            return_payload_len = 0
        else:
            return_payload_len = return_payload_len[0]

        if return_payload_len != 0:
            return_payload = self.serial_read(return_payload_len)
        else:
            return_payload = []

        return report, retval, return_payload

    def close(self):
        print("Killing thread")
        self.thread_run = False
        # wait for it to stop
        while self.thread.is_alive():
            pass
        print("\n\tClosing serial port...\n")
        self.serial.close()


def parse_report(reportID, payload):
    if reportID == 0x22:
        # typedef struct __attribute__((packed)) {
        #     uint32_t frame_num;   //!< Frame number
        #     uint32_t timestamp;  //!< Timestamp when the packet was received
        #     float x;             //!< Accel X value
        #     float y;             //!< Accel Y value
        #     float z;             //!< Accel Z value
        # } accel_norm_t;
        frame_num, timestamp, x, y, z = pu.parse_accel_packet(payload)
        print("   Accel Packet:")
        print("       Frame #: {}".format(frame_num))
        print("     Timestamp: {} ms".format(timestamp))
        print("        X Axis: {}".format(x))
        print("        Y Axis: {}".format(y))
        print("        Z Axis: {}".format(z))

    elif reportID == 0x23:
        # typedef struct __attribute__((packed)) {
        #     uint32_t frame_num;   //!< Frame number
        #     uint32_t timestamp;  //!< Timestamp when the packet was received
        #     float x;             //!< Mag X value
        #     float y;             //!< Mag Y value
        #     float z;             //!< Mag Z value
        # } mag_norm_t;
        frame_num, timestamp, x, y, z = pu.parse_mag_packet(payload)
        print("   Mag Packet:")
        print("      Frame #: {}".format(frame_num))
        print("    Timestamp: {} ms".format(timestamp))
        print("       X Axis: {}".format(x))
        print("       Y Axis: {}".format(y))
        print("       Z Axis: {}".format(z))

    elif reportID == 0x24:
        packed_data = struct.pack("BBBBBBBBBBBBBBBB", *payload)
        accel_pkt_ovrwt, mag_pkt_ovrwt, accel_hw_ovrwt, mag_hw_ovrwt = struct.unpack("=IIII", packed_data)
        print("   LSM303DLHC errors:")
        print("     Accel Packet Overwrites: {}".format(accel_pkt_ovrwt))
        print("       Mag Packet Overwrites: {}".format(mag_pkt_ovrwt))
        print("   Accel Hardware Overwrites: {}".format(accel_hw_ovrwt))
        print("     Mag Hardware Overwrites: {}".format(mag_hw_ovrwt))

    elif reportID == 0x30:
        # Pensel Version
        print("    Pensel v{}.{}".format(payload[0], payload[1]))

    elif reportID == 0x31:
        packed_data = struct.pack("BBBB", *payload)
        timestamp = struct.unpack("I", packed_data)[0]
        print("    Timestamp: {} ms".format(timestamp))

    elif reportID == 0x33:
        print("    Switch State: {}".format(payload[0]))
        print("     Main Button: {}".format("open" if payload[1] else "pressed"))
        print("      Aux Button: {}".format("open" if payload[2] else "pressed"))

    else:
        print("    {}".format(" ".join(["{:0>2X}".format(b) for b in output])))


def parse_inputreport(reportID, payload):
    if reportID == 0x81:
        # typedef struct __attribute__((packed)) {
        #     uint32_t frame_num;   //!< Frame number
        #     uint32_t timestamp;  //!< Timestamp when the packet was received
        #     float x;             //!< Accel X value
        #     float y;             //!< Accel Y value
        #     float z;             //!< Accel Z value
        # } accel_norm_t;
        frame_num, timestamp, x, y, z = pu.parse_accel_packet(payload)
        print("   Accel Packet:")
        print("       Frame #: {}".format(frame_num))
        print("     Timestamp: {} ms".format(timestamp))
        print("        X Axis: {}".format(x))
        print("        Y Axis: {}".format(y))
        print("        Z Axis: {}".format(z))

    elif reportID == 0x82:
        # typedef struct __attribute__((packed)) {
        #     uint32_t frame_num;   //!< Frame number
        #     uint32_t timestamp;  //!< Timestamp when the packet was received
        #     float x;             //!< Mag X value
        #     float y;             //!< Mag Y value
        #     float z;             //!< Mag Z value
        # } mag_norm_t;
        frame_num, timestamp, x, y, z = pu.parse_mag_packet(payload)
        print("   Mag Packet:")
        print("      Frame #: {}".format(frame_num))
        print("    Timestamp: {} ms".format(timestamp))
        print("       X Axis: {}".format(x))
        print("       Y Axis: {}".format(y))
        print("       Z Axis: {}".format(z))


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='Process data from the Pensel.')
    parser.add_argument("reportID", type=str, help="The report ID to query")
    parser.add_argument("payload", nargs='*', type=str, help="An optional payload of bytes")
    parser.add_argument('--baudrate', "-b", dest='baudrate', default=115200,
                        help='Baudrate of the serial data to recieve')
    parser.add_argument('--parsed', default=False, action="store_true",
                        help='Whether or not we should print a parsed version of the report')
    parser.add_argument('--verbose', default=False, action="store_true",
                        help='Whether or not we should print extra debug information')

    args = parser.parse_args()

    def find_ports():
        ports = []
        dev_dir = os.listdir("/dev/")
        for thing in dev_dir:
            if "cu." in thing:
                ports.append(thing)

        return ports

    def choose_port(list_of_ports):
        print("\nPorts:")
        for ind, port in enumerate(list_of_ports):
            print("\t{}: {}".format(ind, port))
        while True:
            try:
                choice = int(raw_input("Which port do you choose? "))
            except Exception:
                print("Invalid choice.")
                continue

            if choice < len(list_of_ports):
                return list_of_ports[choice]
            else:
                print("Invalid index.")

    # port = "/dev/" + choose_port(find_ports())
    port = "/dev/cu.SLAB_USBtoUART"

    with Pensel(port, args.baudrate, args.verbose) as pensel:

        payload = []
        if args.payload is not None:
            payload = []
            for s in args.payload:
                payload.append(int(s, 16))

        reportID = int(args.reportID, 16)
        if args.verbose:
            print("Report ID: {}".format(reportID))
            print("Payload: {}".format(payload))

        retval, output = pensel.send_report(reportID, payload)
        if retval is not None and retval == 0:
            print("Report {:0>2X}:".format(reportID))
            print("")
            if args.parsed is False:
                print("    {}".format(" ".join(["{:0>2X}".format(b) for b in output])))
            else:
                parse_report(reportID, output)
        else:
            print("Report {:0>2X} FAILED. Error Code: {}".format(reportID, retval))
