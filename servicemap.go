package main

import (
	"fmt"
	"net"
	"os"
	"strings"
       )


func main() {
	ServerAddr, err := net.ResolveUDPAddr("udp", ":25446")
	CheckErr(err)
	serverStore := make(map[string]string)
	ServerCLNS, err := net.ListenUDP("udp", ServerAddr)
	CheckErr(err)
	defer ServerCLNS.Close()
	buffer := make([]byte, 1024)
	for {
		n, addr, err := ServerCLNS.ReadFromUDP(buffer)
		fmt.Println("Received from ", addr.IP.String(), ":", string(buffer[0:n]))
		messageParts := strings.Split(string(buffer[0:n]), " ")
		response := ""
		switch messageParts[0] {
                case "PUT":
			serverStore[messageParts[1]] = addr.IP.String() + ":" + messageParts[2]
			response = "OK"
		case "GET":
			response = serverStore[messageParts[1]]
		}
		message := []byte(response)
		ServerCLNS.WriteToUDP(message, addr)
		if err != nil {
                  fmt.Println("Error: ", err)
		}
	    }
}


func CheckErr(err error) {
	if err != nil {
        	fmt.Println(" Error: ", err)
		os.Exit(0)
	}
}
