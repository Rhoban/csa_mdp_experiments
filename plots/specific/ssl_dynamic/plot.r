source("../../plot_tools.r")
library(dplyr)
library(optparse)
library(rjson)

# TODO
# - Add an auto for space size
# - Remove drawing parameters or use them with cmd line
# - Print one out of 'n' entries

# Drawing parameters
# TODO: unit should be specified in [m]
ballSize <- 0.3 # point size -> unit relative to graph size
targetSize <- 5 # point size -> unit relative to graph size

# Convert a data frame with coordinate in world basis to robot basis
robotFromWorld <- function(data, robot_x, robot_y, robot_dir) {
    data.frame(x = robot_x + data$x * cos(robot_dir) - data$y * sin(robot_dir),
               y = robot_y + data$x * sin(robot_dir) + data$y * cos(robot_dir))
}

mkRobot <- function(config, npoints=100){
    tt <- seq(0,2*pi,length.out = npoints)
    xx <- config$collision_radius* cos(tt)
    yy <- config$collision_radius* sin(tt)
    xx[which(xx > config$collision_forward)] <- config$collision_forward
    return(data.frame(x = xx, y = yy))
}

mkRobotWithPose <- function(config, robotX, robotY, robotDir, npoints=100){
    robotFromWorld(mkRobot(config, npoints), robotX, robotY, robotDir)
}

mkRect <- function(center = c(0,0),dx = 1, dy = 1)
{
    xFactor <- c(1,1,-1,-1)
    yFactor <- c(1,-1,-1,1)
    xx <- center[1] + dx * xFactor
    yy <- center[2] + dy * yFactor
    return(data.frame(x = xx, y = yy))
}

mkKickArea <- function(config)
{
    dx <- config$kick_x_limits[2] - config$kick_x_limits[1]
    center <- c(mean(config$kick_x_limits), 0)
    mkRect(center, dx/2, config$kick_y_tol);
}

mkFinishArea <- function(config)
{
    dx <- config$finish_x_limits[2] - config$finish_x_limits[1]
    center <- c(mean(config$finish_x_limits), 0)
    mkRect(center, dx/2, config$finish_y_tol);
}

# Create the triangle going from the robot and including all the acceptable position for the target
# dir_length: the length of the target vector
mkDirOkZoneTarget <- function(kick_tol, robot_x, robot_y, robot_dir, dir_length = 100) {
    nominal_x <- cos(robot_dir) * dir_length
    nominal_y <- sin(robot_dir) * dir_length
    offset_length <- dir_length * tan(kick_tol)
    offset_x <- cos(robot_dir + pi/2) * offset_length
    offset_y <- sin(robot_dir + pi/2) * offset_length
    xx <- c(0,nominal_x + offset_x,nominal_x - offset_x) + robot_x
    yy <- c(0,nominal_y + offset_y,nominal_y - offset_y) + robot_y
    return(data.frame(x = xx, y = yy))
}

# Plot the trajectory of the robot in target referential
vectorPlotTarget <- function(data, variables, outputPath, config)
{
    # Properties
    collisionRadius <- config$collision_radius # in [m]
    collisionForward <- config$collision_forward # in [m]
    dt <- config$dt
    robot_arrow_length <- 0.05 # TODO -> param
    # Computing data in target basis
    c_dir <- cos(data$robot_dir)
    s_dir <- sin(data$robot_dir)
    vecData <- data.frame(
        "robot_dir" = data$robot_dir,
        "target_x" = 0,
        "target_y" = 0,
        "robot_x" = -c_dir * data$target_x + s_dir * data$target_y,
        "robot_y" = -s_dir * data$target_x - c_dir * data$target_y,
        "time" = data$step * dt,
        "kick_dir_tol" = data$kick_dir_tol)
    vecData$robot_end_x <- vecData$robot_x + c_dir * robot_arrow_length
    vecData$robot_end_y <- vecData$robot_y + s_dir * robot_arrow_length
    vecData$ball_x <- vecData$robot_x + c_dir * data$ball_x - s_dir * data$ball_y
    vecData$ball_y <- vecData$robot_y + s_dir * data$ball_x + c_dir * data$ball_y

    # Last entry is used to draw finalStatus
    lastEntry <- tail(vecData,1)

    # Centring everything in position of last ball
    offset_x <- lastEntry$ball_x
    offset_y <- lastEntry$ball_y
    for (prefix in c("robot","robot_end","ball","target"))
    {
        for (suffix in c("x","y")) {
            col <- sprintf("%s_%s",prefix,suffix)
            offset <- ifelse(suffix == "x", offset_x, offset_y)
            vecData[,col] <- vecData[,col] - offset
        }
    }

    # Updating lastEntry
    lastEntry <- tail(vecData,1)
        
    collisionColor <- cbbPalette[1]
    dirOkColor     <- cbbPalette[2]
    kickColor      <- cbbPalette[3]
    stepMinColor   <- cbbPalette[4]
    stepMaxColor   <- cbbPalette[5]
    # plotting
    x_var <- "x"
    y_var <- "y"
    g <- ggplot(vecData)
    # Adding kick area
    okZone <- NULL
    if (config$mode == "Wide") {
        okZone <- robotFromWorld(mkFinishArea(config),
                                 lastEntry$robot_x, lastEntry$robot_y, lastEntry$robot_dir)
    } else if (config$mode %in% c("Finish","Full")) {
        okZone <- robotFromWorld(mkKickArea(config),
                                 lastEntry$robot_x, lastEntry$robot_y, lastEntry$robot_dir)
    }
    g <- g + geom_polygon(aes(x=x,y=y), okZone, size = 0,
                          fill= kickColor, alpha=0.5)
    # Adding direction
    dirOkZone <- mkDirOkZoneTarget(lastEntry$kick_dir_tol, lastEntry$robot_x, lastEntry$robot_y,
                                   lastEntry$robot_dir)
    g <- g + geom_polygon(aes(x=x,y=y), dirOkZone, size = 0,
                          fill= dirOkColor, alpha=0.5)
    # Plot final robot
    collisionData <- mkRobotWithPose(config, lastEntry$robot_x, lastEntry$robot_y, lastEntry$robot_dir)
    g <- g + geom_polygon(aes(x=x,y=y), collisionData , size = 0,
                          fill= collisionColor, alpha=1)
    # plot target
    g <- g + geom_point(aes(x= target_x, y=target_y),
                        shape="x",
                        vecData, size = targetSize)
    toTargetData <- data.frame(x=c(lastEntry$robot_x, lastEntry$target_x),
                               y= c(lastEntry$robot_y, lastEntry$target_y))
    g <- g + geom_path(aes(x=x,y=y), toTargetData)
    # plot robot 
    g <- g + geom_segment(aes(x=robot_x,y=robot_y, xend = robot_end_x, yend = robot_end_y,
                              color = time),
                          arrow = arrow(length = unit(0.05,"cm"))
                          )
    # plot ball
    g <- g + geom_point(aes(x=ball_x,y=ball_y,
                            color = time), size = ballSize)
    # Setting axis
    g <- g + scale_x_continuous(name = "x [m]",
                                breaks = variables[[x_var]][["breaks"]],
                                labels = variables[[x_var]][["labels"]])
    g <- g + scale_y_continuous(name = "y [m]",
                                breaks = variables[[y_var]][["breaks"]],
                                labels = variables[[y_var]][["labels"]])
    g <- g + scale_color_gradient(name="time [s]",
                                  low = stepMinColor, high = stepMaxColor)
    g <- g + coord_cartesian(xlim = variables[[x_var]][["limits"]],
                             ylim = variables[[y_var]][["limits"]])

    # Set theme
    g <- g + theme_bw()
    ggsave(file = outputPath, g,width=5,height=5)
}

vectorPlotRuns <- function(path, variables, options, problem_config)
{
    data <- read.csv(path)
    base <- getBase(path)
    rewards <- aggregate(reward~run, data, sum)
    nb_runs <- nrow(rewards)
    if (options$nb_runs > 0) {
        nb_runs <- min(options$nb_runs,nb_runs)
    }
    if (options$order) {
        rewards <- arrange(rewards, desc(reward))
    }
    lastRunsIdx <- rewards$run#[(nrow(rewards)-nb_runs+1):nrow(rewards)]
    lastRuns <- rewards[lastRunsIdx,]$run
    print(rewards)
    if (!options$no_plot) {
        for (rank in seq(1,length(lastRuns)))
        {
            run <- lastRuns[rank]
            filteredData <- data[which(data$run == run),]
            dst <- sprintf("%slast_vector_plot_%d_run_%d.png", base, rank, run)
            vectorPlotTarget(filteredData, variables, dst, problem_config)
        }
    }
}

option_list <- list(
    make_option(c("-n","--nb_runs"), type="integer", default=0,
                help="Number of runs to plot, 0 means all runs are plotted"),
    make_option(c("-s","--space_size"), type="double", default=1.0,
                help="Size of the space to show on graphs"),
    make_option(c("-o","--order"), action="store_true", default = FALSE,
                help="Order values of rewards from best to worse"),
    make_option("--no_plot", action="store_true", default = FALSE,
                help="Order values of rewards from best to worse")
    )
parser <- OptionParser(usage="%prog [options] <run_logs.csv> <problem.json>",
                       option_list = option_list)
cmd <- parse_args(parser, commandArgs(TRUE), positional_arguments=2)

if (length(args) < 1) {
    cat("Usage: ... <logFiles>\n");
    quit(status=1)
}

problem <- fromJSON(file = cmd$args[2])

if (problem[["class name"]] != "SSLDynamicBallApproach") {
    print("Invalid problem: expecting a SSLDynamicBallApproach problem")
    quit()
}
variables <- list(x      = list(limits = c(-cmd$options$space_size, cmd$options$space_size)),
                  y      = list(limits = c(-cmd$options$space_size, cmd$options$space_size)))
# computing breaks
for (v in names(variables))
{
    min <- variables[[v]][["limits"]][1]
    max <- variables[[v]][["limits"]][2]
    variables[[v]][["breaks"]] = min + (max - min) * seq(0,1,1/4)
}
# computing labels
for (v in names(variables))
{
    breaks <- variables[[v]][["breaks"]]
    variables[[v]][["labels"]] = sapply(breaks, toString)
}



vectorPlotRuns(cmd$args[1], variables, cmd$options, problem$content)

warnings()
