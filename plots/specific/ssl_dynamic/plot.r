source("../../plot_tools.r")

# Drawing parameters
ballSize <- 1.2 # point size -> unit relative to graph size
targetSize <- 5 # point size -> unit relative to graph size

mkCircle <- function(center = c(0,0),diameter = 1, npoints = 100){
    r = diameter / 2
    tt <- seq(0,2*pi,length.out = npoints)
    xx <- center[1] + r * cos(tt)
    yy <- center[2] + r * sin(tt)
    return(data.frame(x = xx, y = yy))
}

mkRobot <- function(robotRadius, collisionForward, npoints=100){
    tt <- seq(0,2*pi,length.out = npoints)
    xx <- robotRadius * cos(tt)
    yy <- robotRadius * sin(tt)
    xx[which(xx > collisionForward)] = collisionForward
    return(data.frame(x = xx, y = yy))
}

mkRect <- function(center = c(0,0),dx = 1, dy = 1)
{
    xFactor <- c(1,1,-1,-1)
    yFactor <- c(1,-1,-1,1)
    xx <- center[1] + dx * xFactor
    yy <- center[2] + dy * yFactor
    return(data.frame(x = xx, y = yy))
}

mkKickArea <- function(kick)
{
    mkRect(kick$center, kick$xTol, kick$yTol);
}



mkDirOkZone <- function(tolerance, x_max) {
    dy <- sin(tolerance) * x_max
    xx <- c(0,x_max,x_max)
    yy <- c(0,dy,-dy)
    return(data.frame(x = xx, y = yy))
}

# Plot the trajectory of the robot in target referential
vectorPlotTarget <- function(data, variables, outputPath)
{
    dt <- 0.05
    robot_arrow_length <- 0.05
    c_dir <- cos(data$robot_dir)
    s_dir <- sin(data$robot_dir)
    vecData <- data.frame(
        "robot_dir" = data$robot_dir,
        "robot_x" = -c_dir * data$target_x - s_dir * data$target_y,
        "robot_y" = s_dir * data$target_x - c_dir * data$target_y,
        "time" = data$step * dt)
    vecData$robot_end_x <- vecData$robot_x + c_dir * robot_arrow_length
    vecData$robot_end_y <- vecData$robot_y + s_dir * robot_arrow_length
    vecData$ball_x <- vecData$robot_x + c_dir * data$ball_x + s_dir * data$ball_y
    vecData$ball_y <- vecData$robot_y - s_dir * data$ball_x + c_dir * data$ball_y
        
    stepMinColor   <- cbbPalette[4]
    stepMaxColor   <- cbbPalette[5]
    # plotting
    x_var <- "x"
    y_var <- "y"
    g <- ggplot(vecData)
    # plot target in robot referential
    g <- g + geom_segment(aes(x=robot_x,y=robot_y, xend = robot_end_x, yend = robot_end_y,
                              color = time),
                          arrow = arrow(length = unit(0.05,"cm"))
                          )
    g <- g + geom_point(aes(x=ball_x,y=ball_y,
                            color = time))
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
vectorPlot <- function(data, variables, outputPath)
{
    # Properties
    collisionRadius <- 0.12 # in [m]
    collisionForward <- 0.09# in [m]
    # Kick
    kick <- list(center = c(0.11, 0),
                 xTol = 0.02,
                 yTol = 0.05)
    kickArea <- mkKickArea(kick);
    
    x_var <- "x"
    y_var <- "y"
    # Calculating temporary variables
    vecData <- data
    # Computing bounded target
    vecData["targetDist"] <- sqrt(vecData$target_x ** 2 + vecData$target_y ** 2)
    vecData["targetDir"] <- atan2(vecData$target_y,vecData$target_x)
    vecData[which(vecData$targetDist > spaceSize),"targetDist"] = spaceSize
    vecData$normTarX <- cos(vecData$targetDir) * vecData$targetDist
    vecData$normTarY <- sin(vecData$targetDir) * vecData$targetDist
    # creating target area and forbidden area (only for last step)
    lastEntry <- tail(vecData,1)
    dirOkZone <- mkDirOkZone(lastEntry[,c("kick_dir_tol")], variables[[x_var]][["limits"]][2])
    collisionData <- mkRobot(collisionRadius, collisionForward)
    dirOkColor      <- cbbPalette[2]
    stepMinColor   <- cbbPalette[4]
    stepMaxColor   <- cbbPalette[5]
    kickColor      <- cbbPalette[3]
    collisionColor <- cbbPalette[1]
    # plotting
    g <- ggplot()
    # plot kickable area
    g <- g + geom_polygon(aes(x=x,y=y), kickArea , size = 0, fill= kickColor
                        , alpha=1)
    # plot dir ok zone
    g <- g + geom_polygon(aes(x=x,y=y), dirOkZone, size = 0,
                          fill= dirOkColor, alpha=0.5)
    # plot collision area
    g <- g + geom_polygon(aes(x=x,y=y), collisionData , size = 0,
                          fill= collisionColor, alpha=1)
    # plot balls in robot referential
    g <- g + geom_point(aes(x = ball_x,y = ball_y,
                            color = step * dt),
                        vecData,
                        size = ballSize)
    # plot target in robot referential
    g <- g + geom_point(aes(x=normTarX,y=normTarY,
                            color = step * dt),
                        shape="x",
                        vecData, size = targetSize)
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

vectorPlotLasts <- function(path, variables, nbRuns = 10)
{
    data <- read.csv(path)
    base <- getBase(path)
    rewards <- aggregate(reward~run, data, sum)
    nbRuns <- min(nrow(rewards),nbRuns)
    lastRunsIdx <- seq(max(data$run) - nbRuns + 1, max(data$run))
    lastRuns <- rewards[lastRunsIdx,]$run
    print(rewards[lastRunsIdx,])
    for (rank in seq(1,length(lastRuns)))
    {
        run <- lastRuns[rank]
        filteredData <- data[which(data$run == run),]
        dst <- sprintf("%slast_vector_plot_%d_run_%d.png", base, rank, run)
        vectorPlotTarget(filteredData, variables, dst)
    }
}


dt <- 0.1

spaceSize <- 2.0 

variables <- list(x      = list(limits = c(-spaceSize, spaceSize)),
                  y      = list(limits = c(-spaceSize, spaceSize)))
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


args <- commandArgs(TRUE)

if (length(args) < 1) {
    cat("Usage: ... <logFiles>\n");
    quit(status=1)
}

for (i in 1:length(args)) {
    path = args[i]
    vectorPlotLasts(path, variables, 50)
}

warnings()
