#include <fenv.h>
#include <core/core.h>
#include <core/com/net/vision_client.h>
#include <core/com/net/vision_analyzer.h>
#include <core/com/net/sim_client.h>
#include <core/constants.h>
#include <core/lambda_task.h>
#include <math/physical_object.h>
#include <base/base.h>
#include <base/filters/tasks/detections_filter.h>
#include <base/filters/tasks/records_analyzer.h>
#include <base/filters/tasks/robot_status_filter.h>

using namespace namec;
using namespace namec::core;
using namespace namec::base;

void configSimulation(SimClient* sim_client, double robot_start_x, double robot_start_y, double robot_start_t)
{
  // move all robot outside the field
  for (int i = 0; i < MAX_ROBOTS_ID; i++)
  {
    sim_client->moveRobot(true, i, (i + 1) * 0.3, -6, M_PI_2, false);
    sim_client->moveRobot(false, i, (i + 1) * -0.3, -6, M_PI_2, false);
  }
  sim_client->moveRobot(false, 0, robot_start_x, robot_start_y, robot_start_t, true);
}

void sigintHandler(int)
{
  getTaskManager().shutdown();
}

int main(int argc, char* argv[])
{
  signal(SIGINT, sigintHandler);
  feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);

  constexpr int CONTROLED_ROBOT_ID = 0;
  getRaw().allies = namec::core::BLUE;  // we play blue

  // construct Simula_tor client and configure starting state
  auto sim_client = new SimClient(false /*we not yellow*/);
  configSimulation(sim_client, 0.0, 0.0, 0.0);

  // Program's pipeline initialization
  getTaskManager().addTask(new VisionClient(DEFAULT_VISION_ADDRESS, DEFAULT_VISION_PORT, ""));
  getTaskManager().addTask(new SslGeometryPacketAnalyzer());
  getTaskManager().addTask(new DetectionPacketAnalyzer());

  // filters + analysers (pos, speed, acc estimations)
  getTaskManager().addTask(
      new filter::DetectionFilter(getRaw().ball_buffer, filter::getRecords().ball, getData().ball, MAX_BALL_SPEED));
  getTaskManager().addTask(new filter::RecordAnalyzer(filter::getRecords().ball, getData().ball));
  getTaskManager().addTask(new filter::DetectionFilter(getRaw().robots_buffers[getRaw().allies][CONTROLED_ROBOT_ID],
                                                       filter::getRecords().robots[getRaw().allies][CONTROLED_ROBOT_ID],
                                                       getData().robots[getRaw().allies][CONTROLED_ROBOT_ID],
                                                       10 /* max robot speed accepted from vision */));
  getTaskManager().addTask(new filter::RecordAnalyzer(filter::getRecords().robots[getRaw().allies][CONTROLED_ROBOT_ID],
                                                      getData().robots[getRaw().allies][CONTROLED_ROBOT_ID]));
  getTaskManager().addTask(new filter::RobotStatusFilter());

  // send message in order to start the ball movement
  getTaskManager().addTask(new LambdaTask([&]() {
    auto ball_start_x = -0.5;
    auto ball_start_y = 0.5;
    auto ball_start_vel_x = 2.0;
    auto ball_start_vel_y = -0.5;
    sim_client->moveBall(ball_start_x, ball_start_y, ball_start_vel_x, ball_start_vel_y);
    return false;  // only send once
  }));

  // analyse and compute new commands
  getTaskManager().addTask(new LambdaTask([&]() {
    // Current state
    const base::math::PhysicalObject& robot = base::getData().robots[getRaw().allies][CONTROLED_ROBOT_ID];
    const base::math::PhysicalObject& ball = base::getData().ball;
    auto v_x = robot.velocity.x;
    auto v_y = robot.velocity.y;
    auto v_theta = robot.velocity.t;
    auto linear_pos_x = robot.x;
    auto linear_pos_y = robot.y;
    auto angular_pos = robot.t;
    auto ball_pos_x = ball.x;
    auto ball_pos_y = ball.y;
    auto ball_vel_x = ball.velocity.x;
    auto ball_vel_y = ball.velocity.y;

    // Compute new command
    // TODO

    // Set new command
    RobotCommand& cmd = getRaw().robot_orders[getRaw().allies][CONTROLED_ROBOT_ID];
    cmd.should_be_send = true;
    cmd.x_linear_velocity = 0.2;
    cmd.y_linear_velocity = 0.0;
    cmd.angular_velocity = 0.0;
    return true;  // return false to stop the task
  }));

  getTaskManager().addTask(sim_client);
  getTaskManager().addTask(new RawReset());

  // execute the pipeline each 10ms
  getTaskManager().run(10ms);

  return 0;
}
