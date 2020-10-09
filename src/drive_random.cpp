#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/LaserScan.h"
#include "nav_msgs/OccupancyGrid.h"
#include "std_msgs/Header.h"
#include "nav_msgs/MapMetaData.h"
#include "std_msgs/Header.h"
#include <time.h>       /* time_t, struct tm, difftime, time, mktime */
#include <thread>   
#include <algorithm> 
#include <math.h>         

void callback();
ros::Publisher pub;
ros::Subscriber subScan;
ros::Subscriber subMap;
bool shouldDodge = false;
bool isTurning;
bool turnLeft;
double Ngray = 0;

bool randomBool()
{
    return rand() % 2;
}

void cleanShutdown()
{
    geometry_msgs::Twist twist;
    twist.linear.x = 0.0; twist.linear.y = 0.0; twist.linear.z = 0.0;
    twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = 0.0;
    pub.publish(twist);
}

void callbackMap(const nav_msgs::OccupancyGrid::ConstPtr& msg)
{
    nav_msgs::MapMetaData info = msg->info;
    int mistakes = 0;
    for(int i=0; i<info.height*info.width; i++)
    {
      //Checken ob der Rahmen geschlossen ist, dadurch, das abgefragt wird, ob neben Pixeln die als grau gekennzeichnet sind(0) Pixel sind, die unbekannt sind(-1). 
        if(double((msg->data[i])==0)&&(double(msg->data[i+1])==-1))
        {
            //Rechts
            //std::cout<<"Karte nicht vollständig!"<<std::endl;

            mistakes++;
        }
        if(double((msg->data[i])==0)&&(double(msg->data[i+1])==-1))
        {
            //Links
            //std::cout<<"Karte nicht vollständig!"<<std::endl;
            mistakes++;
        }
         if(i>info.width)
        {
        if(double((msg->data[i])==0)&&(double(msg->data[i+info.width])==-1))
            {
                //Oben
                //std::cout<<"Karte nicht vollständig!"<<std::endl;
                mistakes++;
            }
        }
        if(i<info.height*info.width-info.width)
        {
            if(double((msg->data[i])==0)&&(double(msg->data[i-info.width])==-1))
            {
                //Unten
                //std::cout<<"Karte nicht vollständig!"<<std::endl;
                
                mistakes++;
            }
        }
        
    }
    std::cout<< "Map Fehler:" << mistakes << std::endl;

    if (mistakes < 30)
    {
        std::cout<<"Karte vollständig"<<std::endl;
        system("rosrun map_server map_saver -f ~/map");
        cleanShutdown();
        exit(0);

        std::cout<<Ngray<<std::endl;
        Ngray=0; 
    }  
}

void callback(const sensor_msgs::LaserScan::ConstPtr& msg)
{
    geometry_msgs::Twist twist;

    // Herausfinden wieviele Daten im angle[] array gespeichert sind: von 0 bis 359
    //int num_readings = std::ceil( (msg->angle_max - msg->angle_min) / msg->angle_increment );
    //std::cout << num_readings << std::endl;
    //std::cout << "Range 350:" <<  msg->ranges[350] << std::endl;


    // Herausfinden welche bei der Ausweichung sinnvoll sind
    /*
    for (int i = 0; i <= 359; i++)
    {
        if (msg->ranges[i] <= 0.3 && msg->ranges[i] >= 0.00001)
        {
            std::cout << i << std::endl;
        } 
    }
    std::cout << "new" << std::endl;
    */

    shouldDodge = false;

    for(int i = 1; i <= 45; i++)
    {
        if (msg->ranges[i] <= 0.2 && msg->ranges[i] >= 0.0001)
        {
            shouldDodge = true;
        }
    }

    for(int i = 315; i <= 359; i++)
    {
        if (msg->ranges[i] <= 0.2 && msg->ranges[i] >= 0.0001)
        {
            shouldDodge = true;
        }
    }

    if (msg->ranges[0] <= 0.4)
    {
        shouldDodge = true;
    }

    if(shouldDodge)
    {
        twist.linear.x = 0.0; twist.linear.y = 0.0; twist.linear.z = 0.0;
        std::cout << randomBool() << std::endl;

        if (isTurning)
       {

       } else
       {
           turnLeft = randomBool();
       }
       
       isTurning = true;

        if(turnLeft)
        {
            twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = 2.0;

        } else 
        {
            twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = -2.0;

        }
    } else 
    {
        twist.linear.x = 0.3; twist.linear.y = 0.0; twist.linear.z = 0.0;
        isTurning = false;
    }
    pub.publish(twist);
}

void launchSlam()
{
    system("roslaunch turtlebot3_slam turtlebot3_slam.launch slam_methods:=gmapping");
}

int main(int argc, char** argv){
    time_t seconds;
    time(&seconds);
    srand((unsigned int) seconds);
    ros::init(argc, argv, "drive_random");
    ros::NodeHandle n;
    pub = n.advertise<geometry_msgs::Twist>("cmd_vel", 10);
    subScan = n.subscribe("/scan", 10, callback);
    subMap = n.subscribe("/map", 1, callbackMap);

    std::thread slamThread (launchSlam); 

    while(ros::ok())
    {
        ros::spin();
    }
 }
