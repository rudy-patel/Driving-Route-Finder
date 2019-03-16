/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Assignment 2 Part 1: Driving Route Finder
Names: Rutvik Patel, Kaden Dreger
ID: 1530012, 1528632
CCID: rutvik, kaden
CMPUT 275 Winter 2018

This program demonstrates an implementation of reading in from a CSV file,
constructing a weighted digraph, and process requests from the user. The user
can specify a latitude and longitude of both start and end points and this
program will find the shortest path and print out the waypoints along the way.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
#include <string>
#include <iostream>
#include <fstream>
#include "dijkstra.h"
#include "digraph.h"
#include "wdigraph.h"
#include "serialport.h"
#include <sstream>
#include <cassert>

bool timeout = false;

SerialPort port("/dev/ttyACM0");
struct Point {
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
The Point struct stores the latitude and longitude of the points to be read in.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    ll lat;  // latitude of the point
    ll lon;  // longitude of the point
};


ll manhattan(const Point& pt1, const Point& pt2) {
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
The manhattan function takes the parameters:
    pt1: the first/starting point
    pt2: the second/end point

It returns the parameters:
    dist: the manhattan distance between the two points

The point of this function is to calculate the manhattan distance between the
two points passed into the function.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    // calculate manhattan distance
    ll dist = abs(pt1.lat - pt2.lat) + abs(pt1.lon - pt2.lon);
    return dist;
}


vector<string> split(string str, char delim) {
    vector<string> temp;
    stringstream ss(str);
    string token;
    while(getline(ss, token, delim)) {
        temp.push_back(token);
    }
    return temp;
}


void readGraph(string filename, WDigraph& graph, unordered_map<int, Point>& points) {
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
The readGraph function takes the parameters:
    filename: name of the CSV file describing the road network
    graph   : an instance of the Wdigraph class
    points  : a mapping b/w vertex ID's and their respective coordinates

It returns the parameters:
    dist: the manhattan distance between the two points

The point of this function is to calculate the manhattan distance between the
two points passed into the function.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    /* method to open and read from files found from:
    cplusplus.com/doc/tutorial/files/             */
    ifstream file;  // file object
    string token;
    char delim = ',';  // the delimeter of choice
    int ID;
    long double coord;
    Point p, p1, p2;
    file.open(filename);  // opening the file
    if (file.is_open()) {  // if the file is open...
        while (getline(file, token, delim)) {
            if (token == "V") {
                getline(file, token, delim);
                ID = stoi(token);
                getline(file, token, delim);
                coord = stold(token) * 100000;
                coord = static_cast<ll>(coord);
                p.lat = coord;  // saving the coordinate to the points
                getline(file, token);
                coord = stold(token) * 100000;
                coord = static_cast<ll>(coord);
                p.lon = coord;  // saving the coordinate to the points
                points[ID] = p;  // insert into the map
            } else if (token == "E") {
                getline(file, token, delim);
                int id1 = stoi(token);
                getline(file, token, delim);
                int id2 = stoi(token);
                getline(file, token);
                string name = token;
                p1 = points[id1];  // get respective point
                p2 = points[id2];  // get respective point
                // calc man dist, pass in saved points
                ll weight = manhattan(p1, p2);  // calculate distance
                graph.addEdge(id1, id2, weight);  // add the weighted edge
            }
        }
    } else {
        /*Error message in case the file is not found. */
        cout << "ERROR. Unable to open the file " << filename << "." << endl;
    }
    file.close();  // closing the file
}


int closestVert(const ll& lat, const ll& lon, unordered_map<int, Point>& p) {
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
The closestVert function takes the parameters:
    lat   : latitude of a given point
    lon   : longitude of a given point
    p     : a mapping b/w vertex ID's and their respective coordinates

It returns the parameters:
    vert  : the resulting vertex with the given lat and lon values

The point of this function is to find the vertex in our point map based on the
passed in lat and lon values.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    Point point;
    ll dist, temp;
    point.lat = lat;  // assign lat parameter to struct
    point.lon = lon;  // assign lon parameter to struct
    int vert = p.begin()->first;  // set starting vertex to the first one
    dist = manhattan(p[vert], point);  // initialize the distance
    for (auto i: p) {
        temp = manhattan(i.second, point);  // calculate temp distance
        if (temp < dist) {  // if that distance is less than previous...
            vert = i.first;  // set the vertex to the current iteration
            dist = temp;  // set the distance to the new lower value
        }
    }
    return vert;  // return resulting vertex
}


bool printWaypoints(unordered_map<int, Point>& p, unordered_map<int, PLI>& tree,
                    int& startVert, int& endVert) {
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
The printWaypoints function takes the parameters:
    tree     : the search tree with respective to the starting vertex
    startVert: starting vertex
    endVert  : end vertex
    p        : a mapping b/w vertex ID's and their respective coordinates

It does not return any parameters.

The point of this function is to print the number of waypoints, along with their
lat and lon values, enroute to the end vertex.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    stack<int> route;
    int vert = endVert;  // starting at the end point
    string ack;
    while (route.top() != startVert) {  // while we have not reached the start
        route.push(vert);  // push the vertex onto the stack
        vert = tree[vert].second;  // set the vertex to the parent of current
    }
    cout << "N " << route.size() << endl;  // print out number of waypoints
    port.writeline("N ");
    port.writeline(to_string(route.size()));
    port.writeline("\n");
    int size = route.size();
    for (int i = 0; i < size; i++) {
        //cin >> ack;  // receive acknowledgement
        ack = port.readline(1);
        if (ack == "A") {
            /*print out the waypoint coordinates*/
            cout << "W " << p[route.top()].lat << " ";
            assert(port.writeline("W "));
            assert(port.writeline(to_string(p[route.top()].lat)));
            assert(port.writeline(" "));
            assert(port.writeline(to_string(p[route.top()].lon)));
            assert(port.writeline("\n"));
            cout << p[route.top()].lon << endl;
            route.pop();  // removing the element from the stack
        } else {
            // timeout
            return true;
        }
    }
    //cin >> ack;  // receive acknowledgement
    ack = port.readline(1);
    if (ack == "A") {
        port.writeline("E\n");
    } else {
        return true;
    }
    //cout << "E" << endl;  // indicating end of request
    return false;  // no timeout
}


int main() {
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
This is the main function of this program, it calls our implementations above,
along with handling some of the input and output functionality.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    WDigraph graph;  // creating instance of Wdigraph
    unordered_map<int, Point> points;
    readGraph("edmonton-roads-2.0.1.txt", graph, points);
    cout << "read graph " << endl;
    while(true) {
        cout << "inside loop" << endl;
        timeout = false;
        while (!timeout) {
            cout << "inside timeout loop" << endl;
            // string splitting method found from: 
            // geeksforgeeks.org/boostsplit-c-library/
            //cin >> r;  // read in R character
            string temp;
            int i = 0;
            temp = port.readline();
            cout << "got here" << endl;
            while (temp[0] != 'R') {
                temp = port.readline(0);
                cout << temp << " "<< i << endl;
                i++;
            }
            cout << "finished" << endl;
            
            /*
            do {
                temp = port.readline(10);
                cout << "iteration: " << i << endl;
                i++;
            } while((temp) == "");
            */
            vector<string> request = split(temp, ' ');  // find citation later...
            cout << "split the input" << endl;
            cout << request[0] << endl;
            if (request[0] == "R") {
                cout << "got an R" << endl;
                ll startLat, startLon, endLat, endLon;
                //cin >> startLat >> startLon >> endLat >> endLon;  // read in the coordinates
                //cout << request[1] << endl;
                startLat = stoll(request[1]);
                startLon = stoll(request[2]);
                endLat = stoll(request[3]);
                endLon = stoll(request[4]);
                int start = closestVert(startLat, startLon, points);  // map to vertex
                int end = closestVert(endLat, endLon, points);  // map to vertex
                unordered_map<int, PLI> heapTree;
                dijkstra(graph, start, heapTree);
                cout << "calculated DICKSTRA" << endl;
                if (heapTree.find(end) == heapTree.end()) {
                    /* handling the 0 case */
                    //cout << "N 0" << endl;
                    assert(port.writeline("N 0\n"));
                    // do we do an E????????
                    //cout << "E" << endl;
                } else {
                    /* print out the waypoints enroute to the destination */
                    timeout = printWaypoints(points, heapTree, start, end);
                    cout << "sent waypoints" << endl;
                }
            }
        }
    }
    return 0;
}