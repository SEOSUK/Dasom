#include "../include/dasom_drone_tf_converter.h"

TFBroadcaster::TFBroadcaster()
: node_handle_("")
{
  /************************************************************
  ** Initialize ROS Subscribers and Clients
  ************************************************************/
  initSubscriber();

  ds_jnt_ = new DasomJoint(8, 8);

  optitrackQuat.resize(7);
  optitrackQuat_lpf.resize(7);

  drone_command.resize(6);
  drone_measured.resize(6);
  global_EE_position_by_drone.resize(6);
}

TFBroadcaster::~TFBroadcaster()
{
  ROS_INFO("Bye TFBroadcaster!");
  ros::shutdown();
}
// double roll, pitch, yaw;
ros::Publisher pubpub;


void TFBroadcaster::drone_command_position_callback(const geometry_msgs::Vector3 & msg)
{
    drone_command[0] = msg.x;
    drone_command[1] = msg.y;
    drone_command[2] = msg.z;
}

void TFBroadcaster::drone_command_attitude_callback(const geometry_msgs::Vector3 & msg)
{
    drone_command[3] = msg.x;
    drone_command[4] = msg.y;
    drone_command[5] = msg.z;
}

void TFBroadcaster::drone_measured_position_callback(const geometry_msgs::Vector3 & msg)
{
    drone_measured[0] = msg.x;
    drone_measured[1] = msg.y;
    drone_measured[2] = msg.z;
}

void TFBroadcaster::drone_measured_attitude_callback(const geometry_msgs::Vector3 & msg)
{
    drone_measured[3] = msg.x;
    drone_measured[4] = msg.y;
    drone_measured[5] = msg.z;
}

void TFBroadcaster::drone_paletrone_converter_desired()
{
    static tf2_ros::StaticTransformBroadcaster br_pose;
    geometry_msgs::TransformStamped transformStamped_pose;

    transformStamped_pose.header.stamp = ros::Time::now();
    transformStamped_pose.header.frame_id = "world";
    transformStamped_pose.child_frame_id = "tf/paletrone_drone_converter_command";
    transformStamped_pose.transform.translation.x = drone_command[0];
    transformStamped_pose.transform.translation.y = drone_command[1];
    transformStamped_pose.transform.translation.z = drone_command[2];
    
    tf::Quaternion quat;

    quat.setRPY(drone_command[3], drone_command[4], drone_command[5]);

    transformStamped_pose.transform.rotation.x = quat.x();
    transformStamped_pose.transform.rotation.y = quat.y();
    transformStamped_pose.transform.rotation.z = quat.z();
    transformStamped_pose.transform.rotation.w = quat.w();
    
    br_pose.sendTransform(transformStamped_pose);
}

void TFBroadcaster::drone_paletrone_converter_measured()
{
    static tf2_ros::StaticTransformBroadcaster br_pose;
    geometry_msgs::TransformStamped transformStamped_pose;

    transformStamped_pose.header.stamp = ros::Time::now();
    transformStamped_pose.header.frame_id = "world";
    transformStamped_pose.child_frame_id = "tf/paletrone_drone_converter_measured";
    transformStamped_pose.transform.translation.x = drone_measured[0];
    transformStamped_pose.transform.translation.y = drone_measured[1];
    transformStamped_pose.transform.translation.z = drone_measured[2];
    
    tf::Quaternion quat;

    quat.setRPY(drone_measured[3], drone_measured[4], drone_measured[5]);

    transformStamped_pose.transform.rotation.x = quat.x();
    transformStamped_pose.transform.rotation.y = quat.y();
    transformStamped_pose.transform.rotation.z = quat.z();
    transformStamped_pose.transform.rotation.w = quat.w();
    
    br_pose.sendTransform(transformStamped_pose);
}


void TFBroadcaster::dasomEEPoseCallback(const geometry_msgs::Twist& msg)
{
    static tf2_ros::StaticTransformBroadcaster br_pose;
    geometry_msgs::TransformStamped transformStamped_pose;

    transformStamped_pose.header.stamp = ros::Time::now();
    transformStamped_pose.header.frame_id = "tf/palletrone_optitrack";
    transformStamped_pose.child_frame_id = "tf/global_EE_pose";
    transformStamped_pose.transform.translation.x = msg.linear.x;
    transformStamped_pose.transform.translation.y = msg.linear.y;
    transformStamped_pose.transform.translation.z = msg.linear.z;
    
    tf::Quaternion quat;

    quat.setRPY(msg.angular.x, msg.angular.y, msg.angular.z);

    transformStamped_pose.transform.rotation.x = quat.x();
    transformStamped_pose.transform.rotation.y = quat.y();
    transformStamped_pose.transform.rotation.z = quat.z();
    transformStamped_pose.transform.rotation.w = quat.w();
    
    br_pose.sendTransform(transformStamped_pose);
}
void TFBroadcaster::initPublisher()
{

}

void TFBroadcaster::initSubscriber()
{                                 
    drone_command_position = node_handle_.subscribe("/pos_d", 10, &TFBroadcaster::drone_command_position_callback, this, ros::TransportHints().tcpNoDelay());
    drone_command_attitude = node_handle_.subscribe("/desired_angle", 10, &TFBroadcaster::drone_command_attitude_callback, this, ros::TransportHints().tcpNoDelay());         
    drone_measured_position = node_handle_.subscribe("/pos", 10, &TFBroadcaster::drone_measured_position_callback, this, ros::TransportHints().tcpNoDelay());
    drone_measured_attitude = node_handle_.subscribe("/angle", 10, &TFBroadcaster::drone_measured_attitude_callback, this, ros::TransportHints().tcpNoDelay());

    dasom_EE_pose_sub_ = node_handle_.subscribe("/dasom/EE_pose", 10, &TFBroadcaster::dasomEEPoseCallback, this); 
   // global_fixed_gimbal_pose_sub_ = node_handle_.subscribe("/dasom/tf/global_fixed_gimbal_EE_pose", 10, &TFBroadcaster::globalFixedGimbalPoseCallback, this, ros::TransportHints().tcpNoDelay());
   // dasom_EE_cmd_sub_ = node_handle_.subscribe("/dasom/test_Pub", 10, &TFBroadcaster::dasomEECmdCallback, this, ros::TransportHints().tcpNoDelay());
}

int main(int argc, char **argv)
{
    ros::init(argc,argv,"dasom_tf2_broadcaster");
    ros::NodeHandle nh;

    TFBroadcaster tf_br_;

    pubpub = nh.advertise<geometry_msgs::TransformStamped>("/herehere", 10);

    ros::Rate loop(120);

    tf_br_.time_i = ros::Time::now().toSec();
    tf_br_.time_f = 0;
    tf_br_.time_loop = 0;

    while(ros::ok())
    {
        tf_br_.time_f = ros::Time::now().toSec();
        tf_br_.time_loop = tf_br_.time_f - tf_br_.time_i;
        tf_br_.time_i = ros::Time::now().toSec();

        // tf_br_.world2palletrone(tf_br_.optitrackQuat, roll, pitch, yaw);

        ros::spinOnce();
        loop.sleep();
    }

    return 0;
}