#! /usr/bin/python
import os

import penselreport as pr


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='Receive input data from the Pensel.')
    parser.add_argument('--baudrate', "-b", dest='baudrate', default=115200,
                        help='Baudrate of the serial data to recieve')
    parser.add_argument('--parsed', default=False, action="store_true",
                        help='Whether or not we should print a parsed version of the report')
    parser.add_argument("--reports", nargs='*', type=str,
                        help="reports to stream (filter out ones we don't care about)")
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

    reports_to_stream = [int(r, 16) for r in args.reports]

    with pr.Pensel(port, args.baudrate, args.verbose) as pi:
        # turn on accel & mag streaming
        retval, response = pi.send_report(0x20, payload=[3])
        while True:
            try:
                pkt = pi.get_packet()
                if pkt:
                    report, retval, payload = pkt
                    if len(reports_to_stream) == 0 or report in reports_to_stream:
                        if retval == 0:
                            if args.parsed:
                                pr.parse_inputreport(report, payload)
                            else:
                                print(pkt)
                        else:
                            print("ERROR: report {} returned retval {}".format(report, retval))
                    elif args.verbose:
                        print("Ignoring report ID {}".format(report))
            except KeyboardInterrupt:
                break
