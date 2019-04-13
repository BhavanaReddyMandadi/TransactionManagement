package main

import "os"
import "net"
import "fmt"
import "bufio"
import "strings"
import "strconv"
import "encoding/binary"

func main() {

	serverInfo := getServerDetails()

	for {
		var userInput string
		fmt.Printf("> ")
		scanner := bufio.NewScanner(os.Stdin)
		if scanner.Scan() {
			userInput = scanner.Text()
		}
		if userInput == "quit" {
                
			break
		}
		conn, err := net.Dial("tcp", serverInfo)
		if err != nil {
                
			fmt.Println("fail!")
			return
		}
		defer conn.Close()
		input := strings.Split(userInput, " ")
		if input[0] == "query" {
               
			op := uint32(1000)
			binary.Write(conn, binary.BigEndian, op)
			accountNumber, _ := strconv.Atoi(input[1])
			accountnum := uint32(accountNumber)
			binary.Write(conn, binary.BigEndian, accountnum)
		}
		if input[0] == "update" {
                
			op := uint32(1001)
			binary.Write(conn, binary.BigEndian, op)
			accountNumber, _ := strconv.Atoi(input[1])
			accountnum := uint32(accountNumber)
			binary.Write(conn, binary.BigEndian, accountnum)
			amount, _ := strconv.ParseFloat(input[2], 32)
			value := float32(amount)
			binary.Write(conn, binary.BigEndian, value)
		}
		buffer := make([]byte, 1024)
		conn.Read(buffer)
		fmt.Printf("%s\n", buffer)
	}
}


func getServerDetails() string {

	addr, err := net.ResolveUDPAddr("udp", "255.255.255.255:25446")
	CheckErr(err)
	LocalAddr, err := net.ResolveUDPAddr("udp", ":0")
	CheckErr(err)
	CLNS, err := net.ListenUDP("udp", LocalAddr)
	CheckErr(err)
	defer CLNS.Close()
	i := 0
	getMessage := "GET CISBANK"
	i++
	getMsgBuffer := []byte(getMessage)
	_, err1 := CLNS.WriteToUDP(getMsgBuffer, addr)
	if err1 != nil {
        
		fmt.Println(getMessage, err1)
	}
	responseBuffer := make([]byte, 1024)
	n, _, err := CLNS.ReadFromUDP(responseBuffer)
	serverInfo := strings.Split(string(responseBuffer[0:n]), ":")
	fmt.Println("Service provided by", serverInfo[0], "at port", serverInfo[1])
	return string(responseBuffer[0:n])
}


func CheckErr(err error) {

	if err != nil {
        
		fmt.Println("Error: ", err)
		os.Exit(0)
	}
}
