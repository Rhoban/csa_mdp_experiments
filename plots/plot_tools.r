library(ggplot2)
library(gridExtra)
library(stringr)

se <- function(x)
{
    sd(x) / (sqrt(length(x)))
}

ci <- function(x)
{
    1.96 * se(x)
}

bootThetaMean <- function(x,i)
{
    mean(x[i])
}

getBase <- function(path)
{
    base = str_extract(path,".*/")
    if (is.na(base)) {
        base = "./"
    }
    return(base);
}

getFileName <- function(path)
{
    base <- getBase(path)
    if (base == "./")
    {
        return(path)
    }
    else
    {
        pathLength <- str_length(path)
        baseLength <- str_length(base)
        return(substr(path,baseLength +1,pathLength))
    }
}

getFilePrefix <- function(path)
{
    file <- getFileName(path)
    return(str_extract(file,"[^.]+"))
}

cbPalette <- c("#000000", "#E69F00")
# From: http://www.cookbook-r.com/Graphs/Colors_(ggplot2)/
cbbPalette <- c("#000000", "#E69F00", "#56B4E9", "#009E73", "#F0E442", "#0072B2", "#D55E00", "#CC79A7")

fillWaiver <- function(variables)
{
    for (v in names(variables))
    {
        for (field in c("limits", "breaks", "labels"))
        {
            if (is.null(variables[[v]][[field]]))
                variables[[v]][[field]] = waiver()
        }
    }
    return(variables)
}

# variables is a named list with [variable_x = ... , variable_y = ... ]
# list can contains parameters such as limits
densityPlot <- function(path, variables)
{
    if (length(names(variables)) < 2)
    {
        print("densityPlot requires at least 2 variables")
        return(NULL)
    }
    # Reading variables and properties
    x_var <- names(variables)[1]
    y_var <- names(variables)[2]
    variables <- fillWaiver(variables)
    # Determining output
    base <- getBase(path)
    filePrefix <- getFilePrefix(path)
    graphPrefix <- sprintf("density_%s_%s", x_var, y_var)
    dst <- sprintf("%s%s_%s.png", base, graphPrefix, filePrefix)

    data <- read.csv(path)
    # printing density_plot
    g <- ggplot(data, aes_string(x = x_var,y = y_var))
    g <- g + geom_point(size=0.5)
    g <- g + stat_density2d(aes(color = ..level..), size=3, alpha=0.7, contour=TRUE)
    g <- g + scale_color_gradientn(colors = cbPalette)
    g <- g + scale_x_continuous(breaks = variables[[x_var]][["breaks"]],
                                labels = variables[[x_var]][["labels"]])
    g <- g + scale_y_continuous(breaks = variables[[y_var]][["breaks"]],
                                labels = variables[[y_var]][["labels"]])
    g <- g + coord_cartesian(xlim = variables[[x_var]][["limits"]],
                             ylim = variables[[y_var]][["limits"]])
    ggsave(dst, width=16, height=9)
}

densityFacetPlot <- function(path, variables, nbGroups, nbRows)
{
    if (length(names(variables)) < 2)
    {
        print("densityFacetPlot requires at least 2 variables")
        return(NULL)
    }
    # Reading variables and properties
    x_var <- names(variables)[1]
    y_var <- names(variables)[2]
    variables <- fillWaiver(variables)
    # Preparing dst
    base <- getBase(path)
    filePrefix <- getFilePrefix(path)
    graphPrefix <- sprintf("density_facet_%s_%s", x_var, y_var)
    dst <- sprintf("%s%s_%s.png", base, graphPrefix, filePrefix)
    # Pretreating data
    data <- read.csv(path)
    groupSize <- ceiling(max(data$run) / nbGroups)
    if (groupSize < 1)
    {
      groupSize <- 1
    }
    names <- c()
    for (i in 1:nbGroups)
    {
        start <- groupSize * (i - 1) + 1
        end <- min(max(data$run),groupSize * i)
        groupName <- paste(start,end,sep = "-")
        names <- append(names, groupName)
    }
    data$group <- factor(names[(data$run-1) %/% groupSize + 1], levels = names)
    # TODO value depending on the number of elements
    x_bandwidth <- max(data[,x_var]) - min(data[,x_var]) / 10000
    y_bandwidth <- max(data[,y_var]) - min(data[,y_var]) / 10000
    bandwidth <- c(x_bandwidth, y_bandwidth)
    # Plotting
    g <- ggplot(data, aes_string(x = x_var,y = y_var,group="group"))
    g <- g + geom_point(size=0.2)
    g <- g + stat_density2d(aes(color = ..level..),
                            size=0.5, alpha=0.7, contour=TRUE,
                            h = bandwidth, n = 200)
    g <- g + scale_color_gradientn(colors = cbPalette)
    g <- g + facet_wrap(~group, nrow = nbRows)
    g <- g + scale_x_continuous(breaks = variables[[x_var]][["breaks"]],
                                labels = variables[[x_var]][["labels"]])
    g <- g + scale_y_continuous(breaks = variables[[y_var]][["breaks"]],
                                labels = variables[[y_var]][["labels"]])
    g <- g + coord_cartesian(xlim = variables[[x_var]][["limits"]],
                             ylim = variables[[y_var]][["limits"]])
    ggsave(dst, width=16,height=9)
}

heatMapFacet <- function(path, variables, nbGroups, nbRows)
{
    if (length(names(variables)) < 2)
    {
        print("heatMapFacet requires at least 2 variables")
        return(NULL)
    }
    # Reading variables and properties
    x_var <- names(variables)[1]
    y_var <- names(variables)[2]
    variables <- fillWaiver(variables)
    # Preparing dst
    base <- getBase(path)
    filePrefix <- getFilePrefix(path)
    graphPrefix <- sprintf("heat_map_facet_%s_%s", x_var, y_var)
    dst <- sprintf("%s%s_%s.png", base, graphPrefix, filePrefix)
    # Pretreating data
    data <- read.csv(path)
    groupSize <- ceiling(max(data$run) / nbGroups)
    if (groupSize < 1)
    {
      groupSize <- 1
    }
    names <- c()
    for (i in 1:nbGroups)
    {
        start <- groupSize * (i - 1) + 1
        end <- min(max(data$run),groupSize * i)
        groupName <- paste(start,end,sep = "-")
        names <- append(names, groupName)
    }
    data$group <- factor(names[(data$run-1) %/% groupSize + 1], levels = names)

    pointSize <- 0.0001 / sqrt(nrow(data))
    # Plotting
    g <- ggplot(data, aes_string(x = x_var,y = y_var,group="group"))
    g <- g + stat_bin2d(bins=floor(sqrt(nrow(data)) / 20))
    if (nrow(data) < 2000)
    {
        g <- g + geom_point(color = "white", alpha = 0.3, size = pointSize)
    }
    g <- g + scale_fill_gradientn(colors = cbPalette, trans="log")
    g <- g + facet_wrap(~group, nrow = nbRows)
    g <- g + scale_x_continuous(breaks = variables[[x_var]][["breaks"]],
                                labels = variables[[x_var]][["labels"]])
    g <- g + scale_y_continuous(breaks = variables[[y_var]][["breaks"]],
                                labels = variables[[y_var]][["labels"]])
    g <- g + coord_cartesian(xlim = variables[[x_var]][["limits"]],
                             ylim = variables[[y_var]][["limits"]])
    ggsave(dst, width=16,height=9)
}

runsPlot <- function(path, variables, max_runs = 49)
{
    if (length(names(variables)) != 1)
    {
        print("runsPlot requires 1 variable")
        return(NULL)
    }
    variables <- fillWaiver(variables)
    y_var <- names(variables)[1]
    # Dst name
    base <- getBase(path)
    filePrefix <- getFilePrefix(path)
    graphPrefix <- paste("runs", y_var, sep ="_")
    dst <- sprintf("%s%s_%s.png", base, graphPrefix, filePrefix)

    data <- read.csv(path)
    if (max(data$run) > max_runs){
        data = data[which(data$run > max(data$run) - max_runs),]
    }
    # printing density_plot
    g <- ggplot(data, aes_string(x="step", y = y_var, group = "run"))
    g <- g + facet_wrap(~run,nrow = 7)
    g <- g + geom_point(size = 0.5, color = "blue")
    g <- g + scale_y_continuous(breaks = variables[[y_var]][["breaks"]],
                                labels = variables[[y_var]][["labels"]])
    g <- g + coord_cartesian(ylim = variables[[y_var]][["limits"]])
    ggsave(dst, width = 16, height = 9)
}

heatMap <- function(path, variables, nbGroups, nbRows)
{
    if (length(names(variables)) < 2)
    {
        print("heatMap requires at least 2 variables")
        return(NULL)
    }
    # Reading variables and properties
    x_var <- names(variables)[1]
    y_var <- names(variables)[2]
    variables <- fillWaiver(variables)
    # Determining output
    base <- getBase(path)
    filePrefix <- getFilePrefix(path)
    graphPrefix <- sprintf("heat_map_%s_%s", x_var, y_var)
    dst <- sprintf("%s%s_%s.png", base, graphPrefix, filePrefix)

    data <- read.csv(path)
    # printing density_plot
    g <- ggplot(data, aes_string(x = x_var,y = y_var))
    g <- g + stat_bin2d(bins=20)
    g <- g + geom_point(size = 0.3, alpha = 0.3, color = "white")
    g <- g + scale_fill_gradientn(colors = cbPalette, trans="log")
    g <- g + scale_x_continuous(breaks = variables[[x_var]][["breaks"]],
                                labels = variables[[x_var]][["labels"]])
    g <- g + scale_y_continuous(breaks = variables[[y_var]][["breaks"]],
                                labels = variables[[y_var]][["labels"]])
    g <- g + coord_cartesian(xlim = variables[[x_var]][["limits"]],
                             ylim = variables[[y_var]][["limits"]])
    ggsave(dst, width=16, height=9)
}

runsSplitPlot <- function(path, variables, runsByGraph, firstRun = 1)
{
    variables <- fillWaiver(variables)
    data <- read.csv(path)
    nbGraphs <- ceiling((max(data$run) - 1) / runsByGraph)
    firstGraph <- ceiling(firstRun / runsByGraph)
    nbVariables <- length(variables)
    print(firstGraph)
    print(nbGraphs)
    for (i in firstGraph:nbGraphs)
    {
        base <- getBase(path)
        filePrefix <- getFilePrefix(path)
        graphPrefix <- sprintf("traj_part_%03d", i)
        dst <- sprintf("%s%s_%s.png", base, graphPrefix, filePrefix)
        # Which runs are concerned
        runMin <- 1 + (i-1) * runsByGraph
        runMax <- i * runsByGraph
        # Filtering data
        groupData <- data[which(data$run >= runMin & data$run <= runMax),]
        # Creating internal plots
        plots <- list()
        for (v in names(variables))
        {
            g <- ggplot(groupData, aes_string(x="step", y = v, group = "run"))
            g <- g + facet_wrap(~run,nrow = 1)
            g <- g + geom_point(size = 0.5, color = "black")
            g <- g + geom_line(size = 0.2, color = "black")
            g <- g + scale_y_continuous(breaks = variables[[v]][["breaks"]],
                                        labels = variables[[v]][["labels"]])
            g <- g + coord_cartesian(ylim = variables[[v]][["limits"]])
            plots <- c(plots,list(g))
        }
#        do.call(grid.arrange, c(plots, nrow = nbVariables))
        finalG <- arrangeGrob(grobs=plots,nrow=nbVariables)
        ggsave(file=dst, finalG, width = 16, height=9)
    }
}

plotBestRuns <- function(path, variables, nbRuns = 10, inverted=FALSE)
{
    data <- read.csv(path)
    base <- getBase(path)
    rewards <- aggregate(reward~run, data, sum);
    entryOrder <- order(-rewards$reward)
    namePrefix <- "best"
    if (inverted) {
        entryOrder <- order(rewards$reward)
        namePrefix <- "worst"
    }
    bestRuns <- rewards[entryOrder,][seq(1,nbRuns),]$run
    print(rewards[bestRuns,])
    for (rank in seq(1,length(bestRuns)))
    {
        run <- bestRuns[rank]
        filteredData <- data[which(data$run == run),]
        dst <- sprintf("%s%s_runs_%d_run_%d.png", base, namePrefix, rank, run)
        # Creating internal plots
        plots <- list()
        for (v in names(variables))
        {
            g <- ggplot(filteredData, aes_string(x="step", y = v))
            g <- g + geom_point(size = 0.5, color = "black")
            g <- g + geom_line(size = 0.2, color = "black")
            g <- g + scale_y_continuous(breaks = variables[[v]][["breaks"]],
                                        labels = variables[[v]][["labels"]])
            g <- g + coord_cartesian(ylim = variables[[v]][["limits"]])
            plots <- c(plots,list(g))
        }
#        do.call(grid.arrange, c(plots, nrow = nbVariables))
        finalG <- arrangeGrob(grobs=plots,nrow=length(variables))
        ggsave(file=dst, finalG, width = 16, height=9)
    }
}



density1DPlot <- function(path, variables)
{
    if (length(names(variables)) != 1)
    {
        print("runsPlot requires 1 variable")
        return(NULL)
    }
    variables <- fillWaiver(variables)
    x_var <- names(variables)[1]
    # Determining output
    base <- getBase(path)
    filePrefix <- getFilePrefix(path)
    graphPrefix <- sprintf("density_%s", x_var)
    dst <- sprintf("%s%s_%s.png", base, graphPrefix, filePrefix)
    # Reading data
    data <- read.csv(path)
    # Preparing graph
    g <- ggplot(data, aes_string(x = x_var))
    g <- g + geom_bar(size = 0.2, color = "black")
    g <- g + stat_density()
#    g <- g + scale_x_continuous(breaks = variables[[x_var]][["breaks"]],
#                                labels = variables[[x_var]][["labels"]])
#    g <- g + coord_cartesian(xlim = variables[[x_var]][["limits"]])
    ggsave(dst, width=16, height=9)
}

# params is a list of named parameters
rewardsPlot <- function(paths, params = list())
{
    if (length(paths) < 1)
    {
        print("rewardsPlot requires at least 1 log")
        return(NULL)
    }
    # Mixing data
    files <- c()
    runs <- c()
    rewards <- c()
    for (path in paths) {
        data <- read.csv(path)
        filename <- getFilePrefix(path)
        files <- append(files, rep(path, nrow(data)))
        runs <- append(runs, data$run)
        rewards <- append(rewards, data$reward)
    }
    # preparing data
    print(sprintf("files   : %d", length(files  )))
    print(sprintf("runs    : %d", length(runs   )))
    print(sprintf("rewards : %d", length(rewards)))
    data <- data.frame(file = files, run = runs, reward = rewards)
    avgData <- aggregate(reward~file, data, FUN=mean)
    print(avgData)
    # Preparing graph
#    g <- ggplot(data, aes(x = reward, color=file,group = file))
    g <- ggplot(data, aes(x = reward))
#    g <- g + geom_bar(size = 0.2, color = "black")
    g <- g + geom_density(size=0.5,alpha=1)#geom = "line")
#    g <- g + scale_x_continuous(breaks = variables[[x_var]][["breaks"]],
#                                labels = variables[[x_var]][["labels"]])
#    g <- g + geom_vline(data=avgData,
#                        mapping=aes(xintercept=reward, color=file),
#                        size=1.5,alpha=0.6))
    g <- g + geom_vline(data=avgData,
                        xintercept=min(avgData$reward),
                        size=0.2,alpha=1)
    g <- g + geom_vline(data=avgData,
                        xintercept=max(avgData$reward),
                        size=0.2,alpha=1)
    g <- g + geom_vline(xintercept=mean(data$reward),
                        size=1,alpha=1)
    g <- g + coord_cartesian(xlim = params$limits)
    ggsave("rewardGraph.png", width=8, height=4.5)
    print(sprintf("mean reward: %f", mean(data$reward)))
    print(data[which(data$reward == min(data$reward)),])
    print(data[which(data$reward == max(data$reward)),])
}

# Take a reward_logs file as parameter
rewardBarPlot <- function(path, nbGroups = 5, groupColumn="run", plotColumn="reward")
{
    data <- read.csv(path)
    base <- getBase(path)
    dst <- sprintf("%s%s_barplot.png", base, plotColumn)
    # Computing run rewards
    nbElements <- max(data[,groupColumn])
    groupSize <- ceiling(nbElements / nbGroups)
    data$last  <- ceiling(data[,groupColumn] / groupSize) * groupSize
    data$first <- data$last - groupSize + 1
    # Special case for the last interval
    data$last[which(data$last > nbElements)] <- nbElements
    # Setting up group names
    nbDigits <- ceiling(log10(max(data[,groupColumn])))
    isolatedFormat <- sprintf("%%0%dd",nbDigits)
    rangeFormat <- sprintf("%s-%s",isolatedFormat,isolatedFormat)
    data$group <- sprintf(rangeFormat, data$first, data$last)
    isolated <- which(data$first == data$last)
    data[isolated, "group"] <- sprintf(isolatedFormat, data[isolated, "first"]) 
    # Computing mean from group
    groupValues <- do.call(data.frame, aggregate(formula(paste0(plotColumn,"~group")),
                                                 data,
                                                 function(x) c(mean = mean(x),
                                                               sd = sd(x),
                                                               se = se(x),
                                                               ci = ci(x))))
    print(groupValues)
    # Plotting means
    meanStr <- sprintf("%s.mean", plotColumn)
    ciStr <- sprintf("%s.ci", plotColumn)
    yMinStr <- sprintf("%s-%s", meanStr, ciStr)
    yMaxStr <- sprintf("%s+%s", meanStr, ciStr)
    g <- ggplot(groupValues, aes_string(x="group", y=meanStr))
    g <- g + geom_point()
    g <- g + geom_errorbar(mapping = aes_string(x= "group",
                                                ymin=yMinStr,
                                                ymax=yMaxStr))
    g <- g + theme(axis.text.x  = element_text(angle=90, vjust=0.5, size=12))
    g <- g + labs(x = groupColumn)
    ggsave(dst)
}

# Take a reward_logs file as parameter
discRewardBarPlot <- function(path, nbGroups = 5, groupColumn="run")
{
    rewardBarPlot(path, nbGroups, groupColumn, "disc_reward")
}

timePlot <- function(path)
{
    data <- read.csv(path)
    base <- getBase(path)
    dst <- sprintf("%stime_by_type.png", base)
    plotData <- aggregate(time~type,data,sum)
    plotData$timeH <- plotData$time / 3600
    plotData$timeRatio <- 100 * plotData$time / sum(plotData$time)
    print(plotData)
    g <- ggplot(plotData, aes(x=type, y=timeH))
    g <- g + geom_bar(stat="identity")
    ggsave(dst, width=8, height=4.5)
}

compareRunCosts <- function(paths, costColumn="reward", invertCostSign=TRUE)
{
    data <- data.frame(run = integer(),
                       cost = double(),
                       path = character(),
                       stringsAsFactors=FALSE)
    for (path in paths)
    {
        pathData <- read.csv(path)
        pathData$path <- path
        costs <- pathData[,costColumn]
        if (invertCostSign) { costs <- -costs }
        tmpData <- data.frame(run = pathData$run,
                              cost = costs,
                              path = rep(path,nrow(pathData)),
                              stringsAsFactors=FALSE)
        data <- rbind(data, tmpData) 
    }
    dst <- sprintf("compare_%s.png", costColumn)
    
    g <- ggplot(data, aes(x=run, y=cost, group=path, color=path))
    g <- g + geom_point(size=0.2)
#    g <- g + stat_summary(fun.y = "min", fun.ymin = "min", fun.ymax= "min", size= 0.3, geom = "crossbar")
#    g <- g + scale_x_log10()
    g <- g + scale_y_log10()
    ggsave(dst)
}


# TODO disable group notion if nbGroups is negative
compareCosts <- function(paths, groupColumn="run", costColumn="reward",
                         invertCostSign=TRUE, nbGroups=10,
                         costLogScale=FALSE, displayCI=FALSE)
{
    data <- data.frame(groupIndex = integer(),
                       cost = double(),
                       path = character(),
                       stringsAsFactors=FALSE)
    for (path in paths)
    {
        pathData <- read.csv(path)
        pathData$path <- path
        costs <- pathData[,costColumn]
        if (invertCostSign) { costs <- -costs }
        tmpData <- data.frame(groupIndex = pathData[,groupColumn],
                              cost = costs,
                              path = rep(path,nrow(pathData)),
                              stringsAsFactors=FALSE)
        data <- rbind(data, tmpData) 
    }
    dst <- sprintf("compare_%s_%s.png", groupColumn, costColumn)
    
    # Getting group sizes
    nbElements <- max(data$groupIndex)
    groupSize <- ceiling(nbElements / nbGroups)
    data$last  <- ceiling(data$groupIndex / groupSize) * groupSize
    data$first <- data$last - groupSize + 1
    # Special case for the last interval
    data$last[which(data$last > nbElements)] <- nbElements
    # Setting up group names
    nbDigits <- floor(log10(nbElements)) + 1
    isolatedFormat <- sprintf("%%0%dd",nbDigits)
    rangeFormat <- sprintf("%s-%s",isolatedFormat,isolatedFormat)
    data$group <- sprintf(rangeFormat, data$first, data$last)
    isolated <- which(data$first == data$last)
    data[isolated, "group"] <- sprintf(isolatedFormat, data[isolated, "first"]) 
    # Computing mean from group
    groupValues <- do.call(data.frame, aggregate(cost~group + path,
                                                 data,
                                                 function(x) c(mean = mean(x),
                                                               sd = sd(x),
                                                               se = se(x),
                                                               ci = ci(x))))
    print(groupValues)

    # Get min Line for each group
    minCostData <- do.call(data.frame, aggregate(cost.mean~path,
                                                 groupValues,
                                                 function(x) c(min = min(x))))

    print(minCostData)
    g <- ggplot(groupValues, aes_string(x="group", y="cost.mean",
                                        ymin = "cost.mean - cost.sd",
                                        ymax = "cost.mean + cost.sd",
                                        group="path", color="path", fill="path"))
    if (displayCI) { g <- g + geom_ribbon(size=0,alpha = 0.3) }
    g <- g + geom_point(size=2)
    g <- g + geom_line(size=0.5)
    g <- g + geom_hline(data = minCostData,
                        mapping = aes_string(yintercept="cost.mean", color="path"),
                        size=0.5, linetype="longdash")
    if (costLogScale) { g <- g + scale_y_log10() }
    g <- g + theme(axis.text.x  = element_text(angle=90, vjust=0.5, size=8))
    g <- g + theme(legend.position="bottom", legend.direction="vertical", legend.text  = element_text(size=10))
    g <- g + labs(x = groupColumn)
    g <- g + scale_colour_manual(values=cbbPalette)
    ggsave(dst)
}

# Each arg starting by -- specifies the beginning of a category:
# Ex: --cat1 path1 path2 --cat2 path3 path4 path5 ...
# Results is a named list with such as:
# result[["cat1"]] = c(path1, path2)
# result[["cat2"]] = c(path3, path4, path5)
argsToCategories <- function(args)
{
    result <- list()
    category <- "Unknown"
    for (arg in args) {
        if (str_length(arg) > 2 && str_sub(arg,1,2) == "--") {
            category = str_sub(arg,3)
            result[[category]] = c()
        }
        else {
            result[[category]] = c(result[[category]], arg)
        }
    }
    return(result)
}

# Consider all folders in 'path' as a category
# - Each category will contain all the files inside its own path corresponding to the pattern
autoCategories <- function(path, pattern)
{
    print("TEST")
    cat_paths <- list.dirs(path = path, full.names = TRUE, recursive = FALSE)
    print(cat_paths)
    cat <- basename(cat_paths)
    print(cat)
    result <- list()
    for (idx in 1:length(cat_paths)) {
        result[[cat[idx]]] <- list.files(path = cat_paths[idx], full.names = TRUE, pattern = pattern, recursive = TRUE)
    }
    return(result)
}

# Merge all the data coming from multiple categories, each category containing
# the path to multiple csv files.
# All the csv files should have the same columns.
# The data returned will contain two additional columns, the name of the category
# and the path to the file containing the data
gatherCategories <- function(categories)
{
    data <- NULL
    for (catName in names(categories))
    {
        for (path in categories[[catName]])
        {
            pathData <- read.csv(path)
            # Skip empty files
            if (nrow(pathData) == 0)
                next
            pathData$category <- catName
            pathData$path <- path
            if (is.null(data)) {
                data <- pathData
            } else {
                data <- rbind(data, pathData)
            }
        }
    }
    return(data)
}

# categories is the result of previous value
compareCostsByCategories <- function(categories, groupColumn="run", costColumn="reward",
                                     invertCostSign=TRUE, nbGroups=10,
                                     costLogScale=FALSE, displayCI=TRUE,
                                     hlines=c())
{
    # STEP1 : get all data in the same dataframe
    data <- data.frame(groupIndex = integer(),
                       cost = double(),
                       path = character(),
                       category = character(),
                       stringsAsFactors=FALSE)
    for (catName in names(categories))
    {
        for (path in categories[[catName]])
        {
            pathData <- read.csv(path)
            pathData$path <- path
            costs <- pathData[,costColumn]
            if (invertCostSign) { costs <- -costs }
            tmpData <- data.frame(groupIndex = pathData[,groupColumn],
                                  cost = costs,
                                  path = rep(path,nrow(pathData)),
                                  category = rep(catName,nrow(pathData)),
                                  stringsAsFactors=FALSE)

            # If group has several member, average their cost
            # (e.g. each policy has several run)
            tmpData <- aggregate(cost~groupIndex + path + category,
                                 tmpData,
                                 function(x) c(mean = mean(x)))
            
            # Check consistency of data
            if (length(unique(tmpData$groupIndex)) != nrow(tmpData)) {
                print("Misformated data: duplicate of groupIndex!")
                quit(status=-1)
            }
            
            # For each groupIndex, consider the lowest cost which has been reached
            for (groupIndex in sort(unique(tmpData$groupIndex))) {
                current <- which(tmpData$groupIndex == groupIndex)
                considered <- which(tmpData$groupIndex <= groupIndex)
                tmpData[current, "cost"] = min(tmpData[considered,"cost"])
            }
            data <- rbind(data, tmpData)
        }
    }

    # Getting group sizes
    nbElements <- max(data$groupIndex)
    groupSize <- ceiling(nbElements / nbGroups)
    data$last  <- ceiling(data$groupIndex / groupSize) * groupSize
    data$first <- data$last - groupSize + 1
    # Special case for the last interval
    data$last[which(data$last > nbElements)] <- nbElements
    # Setting up group names
    nbDigits <- floor(log10(nbElements)) + 1
    isolatedFormat <- sprintf("%%0%dd",nbDigits)
    rangeFormat <- sprintf("%s-%s",isolatedFormat,isolatedFormat)
    data$group <- sprintf(rangeFormat, data$first, data$last)
    isolated <- which(data$first == data$last)
    data[isolated, "group"] <- sprintf(isolatedFormat, data[isolated, "first"])

    # Group if necessary
    pathValues <- aggregate(cost~group + path + category,
                            data,
                            function(x) c(mean = mean(x)))

    # Now getting average, sd and se for each category
    catValues <- do.call(data.frame, aggregate(cost~group + category,
                                               pathValues,
                                               function(x) c(mean = mean(x),
                                                             sd = sd(x),
                                                             se = se(x),
                                                             ci = ci(x))))
    catValues[,"cost.lci"] <- catValues[,"cost.mean"] - catValues["cost.ci"]
    catValues[,"cost.uci"] <- catValues[,"cost.mean"] + catValues["cost.ci"]
    
    # When using log scale, forbid negative values
    if (costLogScale) {catValues[,"cost.lci"] <- pmax(1,catValues[,"cost.lci"]) }
    
    
    #print(catValues)

    g <- ggplot(catValues, aes_string(x="group", y="cost.mean",
                                      ymin = "cost.lci",
                                      ymax = "cost.uci",
                                      group="category", color="category", fill="category"))
    if (length(hlines) > 0) {
        g <- g + geom_hline(yintercept=hlines, linetype="dashed")
    }
    if (displayCI) { g <- g + geom_ribbon(size=0,alpha = 0.1) }
    #if (displayCI) { g <- g + geom_errorbar(size=0.5)}
    g <- g + geom_point(size=2)
    g <- g + geom_line(size=0.5)
    if (costLogScale) { g <- g + scale_y_log10() }
    g <- g + theme(axis.text.x  = element_text(angle=90, vjust=0.5, size=8))
    g <- g + theme(legend.position="right", legend.direction="vertical", legend.text  = element_text(size=10))
    g <- g + labs(x = groupColumn)
    g <- g + scale_colour_manual(values=cbbPalette)
    g <- g + scale_fill_manual(values=cbbPalette)
    # Saving graph
    dst <- sprintf("compare_%s_%s.png", groupColumn, costColumn)
    ggsave(dst,width=16,height=9)
}


# TODO disable group notion if nbGroups is negative
compareCosts <- function(paths, groupColumn="run", costColumn="reward",
                         invertCostSign=TRUE, nbGroups=10,
                         costLogScale=FALSE, displayCI=FALSE)
{
    data <- data.frame(groupIndex = integer(),
                       cost = double(),
                       path = character(),
                       stringsAsFactors=FALSE)
    for (path in paths)
    {
        pathData <- read.csv(path)
        pathData$path <- path
        costs <- pathData[,costColumn]
        if (invertCostSign) { costs <- -costs }
        tmpData <- data.frame(groupIndex = pathData[,groupColumn],
                              cost = costs,
                              path = rep(path,nrow(pathData)),
                              stringsAsFactors=FALSE)
        data <- rbind(data, tmpData) 
    }
    dst <- sprintf("compare_%s_%s.png", groupColumn, costColumn)
    
    # Getting group sizes
    nbElements <- max(data$groupIndex)
    groupSize <- ceiling(nbElements / nbGroups)
    data$last  <- ceiling(data$groupIndex / groupSize) * groupSize
    data$first <- data$last - groupSize + 1
    # Special case for the last interval
    data$last[which(data$last > nbElements)] <- nbElements
    # Setting up group names
    nbDigits <- floor(log10(nbElements)) + 1
    isolatedFormat <- sprintf("%%0%dd",nbDigits)
    rangeFormat <- sprintf("%s-%s",isolatedFormat,isolatedFormat)
    data$group <- sprintf(rangeFormat, data$first, data$last)
    isolated <- which(data$first == data$last)
    data[isolated, "group"] <- sprintf(isolatedFormat, data[isolated, "first"]) 
    # Computing mean from group
    groupValues <- do.call(data.frame, aggregate(cost~group + path,
                                                 data,
                                                 function(x) c(mean = mean(x),
                                                               sd = sd(x),
                                                               se = se(x),
                                                               ci = ci(x))))
    #print(groupValues)

    # Get min Line for each group
    minCostData <- do.call(data.frame, aggregate(cost.mean~path,
                                                 groupValues,
                                                 function(x) c(min = min(x))))

    #print(minCostData)
    g <- ggplot(groupValues, aes_string(x="group", y="cost.mean",
                                        ymin = "cost.mean - cost.ci",
                                        ymax = "cost.mean + cost.ci",
                                        group="path", color="path", fill="path"))
    if (displayCI) { g <- g + geom_ribbon(size=0,alpha = 0.3) }
    g <- g + geom_line(size=0.5)
    g <- g + geom_point(size=2)
    g <- g + geom_hline(data = minCostData,
                        mapping = aes_string(yintercept="cost.mean", color="path"),
                        size=0.5, linetype="longdash")
    if (costLogScale) { g <- g + scale_y_log10() }
    g <- g + theme(axis.text.x  = element_text(angle=90, vjust=0.5, size=8))
    g <- g + theme(legend.position="right", legend.direction="vertical", legend.text  = element_text(size=10))
    g <- g + labs(x = groupColumn)
    g <- g + scale_colour_manual(values=cbbPalette)
    ggsave(dst)
}

# categories is the result of previous value
compareTimeByCategories <- function(categories,
                                    nbGroups=10, timeLogScale=FALSE, displayCI=TRUE)
{
    # STEP1 : get all data in the same dataframe
    data <- data.frame(groupIndex = integer(),
                       time = double(),
                       cumulatedTime = double(),
                       path = character(),
                       category = character(),
                       stringsAsFactors=FALSE)
    for (catName in names(categories))
    {
        for (path in categories[[catName]])
        {
            pathData <- read.csv(path)
            pathData$path <- path
            tmpData <- data.frame(groupIndex = pathData$run,
                                  time = pathData$time,
                                  path = rep(path,nrow(pathData)),
                                  category = rep(catName,nrow(pathData)),
                                  stringsAsFactors=FALSE)
            # Aggregate time for each log separately
            tmpData <- aggregate(time~groupIndex + path + category, tmpData, sum)
            # Cumulate time with previous runs
            tmpData$cumulatedTime = tmpData$time
            for (groupIndex in sort(unique(tmpData$groupIndex))) {
                current <- which(tmpData$groupIndex == groupIndex)
                considered <- which(tmpData$groupIndex <= groupIndex)
                tmpData[current, "cumulatedTime"] = sum(tmpData[considered,"time"])
            }
            data <- rbind(data, tmpData)
        }
    }

    # Getting group sizes
    nbElements <- max(data$groupIndex)
    groupSize <- ceiling(nbElements / nbGroups)
    data$last  <- ceiling(data$groupIndex / groupSize) * groupSize
    data$first <- data$last - groupSize + 1
    # Special case for the last interval
    data$last[which(data$last > nbElements)] <- nbElements
    # Setting up group names
    nbDigits <- floor(log10(nbElements)) + 1
    isolatedFormat <- sprintf("%%0%dd",nbDigits)
    rangeFormat <- sprintf("%s-%s",isolatedFormat,isolatedFormat)
    data$group <- sprintf(rangeFormat, data$first, data$last)
    isolated <- which(data$first == data$last)
    data[isolated, "group"] <- sprintf(isolatedFormat, data[isolated, "first"])

    # ComputingMean for each path
    pathValues <- do.call(data.frame, aggregate(cumulatedTime~group + path + category,
                                                data,
                                                function(x) c(mean = mean(x))))
    # Now getting average, sd and se for each category
    catValues <- do.call(data.frame, aggregate(cumulatedTime~group + category,
                                               pathValues,
                                               function(x) c(mean = mean(x),
                                                             sd = sd(x),
                                                             se = se(x),
                                                             ci = ci(x))))
    #print(catValues)
    
    g <- ggplot(catValues, aes_string(x="group", y="cumulatedTime.mean",
                                      ymin = "cumulatedTime.mean - cumulatedTime.ci",
                                      ymax = "cumulatedTime.mean + cumulatedTime.ci",
                                      group="category", color="category", fill="category"))
    if (displayCI) { g <- g + geom_ribbon(size=0,alpha = 0.3) }
    g <- g + geom_point(size=2)
    g <- g + geom_line(size=0.5)
    if (timeLogScale) { g <- g + scale_y_log10() }
    g <- g + theme(axis.text.x  = element_text(angle=90, vjust=0.5, size=8))
    g <- g + theme(legend.position="right", legend.direction="vertical", legend.text  = element_text(size=10))
    g <- g + labs(x = "run")
    g <- g + labs(y = "Cumulated time")
    g <- g + scale_colour_manual(values=cbbPalette)
    g <- g + scale_fill_manual(values=cbbPalette)
    # Saving graph
    ggsave("compare_cumulated_time.png",width=16,height=9)
}

compareCostEvolutionByCategories <- function(categories)
{
    # STEP1 : get all data in the same dataframe
    data <- data.frame(elapsedTime = double(),
                       cost = double(),
                       category = character(),
                       stringsAsFactors=FALSE)
    for (catName in names(categories))
    {
        for (path in categories[[catName]])
        {
            pathData <- read.csv(path)
            pathData$path <- path
            tmpData <- data.frame(elapsedTime = pathData$elapsed_time,
                                  cost = -pathData$reward,
                                  category = rep(catName,nrow(pathData)),
                                  stringsAsFactors=FALSE)
            data <- rbind(data, tmpData)
        }
    }
    # STEP2 : plot
    g <- ggplot(data, aes_string(x="elapsedTime", y="cost",
                                 group="category", color="category", fill="category"))
#    g <- g + geom_point(size=0.05)
    g <- g + stat_density2d(aes(size= ..level..), alpha=0.5, contour=TRUE)
#    g <- g + scale_y_continuous(limits=c(5,25))
    ggsave("cost_evolution.png",width=16,height=9)
}

# Use reward_logs files
cmpCostByCat <- function(categories)
{
    # STEP 1 get all data in the same dataframe
    data <- data.frame(cost = double(),
                       category = character(),
                       stringsAsFactors=FALSE)
    for (catName in names(categories))
    {
        for (path in categories[[catName]])
        {
            pathData <- read.csv(path)
            tmpData <- data.frame(cost = -pathData$reward,
                                  category = rep(catName,nrow(pathData)),
                                  stringsAsFactors=FALSE)
            data <- rbind(data, tmpData)
        }
    }
    # STEP2: aggregate
    catValues <- do.call(data.frame, aggregate(cost~category,
                                               data,
                                               function(x) c(mean = mean(x),
                                                             sd = sd(x),
                                                             se = se(x),
                                                             ci = ci(x))))
    print(catValues)
    # STEP3: plot
    g <- ggplot(catValues, aes_string(x="category", y="cost.mean",
                                      ymin = "cost.mean - cost.ci",
                                      ymax = "cost.mean + cost.ci"))
    g <- g + geom_bar(stat="identity")
    g <- g + geom_errorbar(color='red',size=1)
    ggsave("cmpCostByCat.png")   
}

drawRunDistributions <- function(categories, variables, nbVerticalBins = 10)
{
    # STEP 1 get all data in the same dataframe
    data <- NULL
    for (catName in names(categories))
    {
        for (path in categories[[catName]])
        {
            pathData <- read.csv(path)
            pathData$category = catName
            if (is.null(data)) {
                data <- pathData
            }
            else {
                data <- rbind(data, pathData)
            }
        }
    }
    # STEP 2: plot graph
    plots <- list()
    for (v in names(variables))
    {
        g <- ggplot(data, aes_string(x="step", y = v, group="category"))
        g <- g + geom_point(size = 0.2)
        g <- g + geom_bin2d(bins = c(max(data$step) + 1, nbVerticalBins), drop=TRUE)
        #g <- g + geom_line(size = 0.2, color = "black")
        g <- g + facet_wrap(~category,nrow =1)
        g <- g + scale_y_continuous(breaks = variables[[v]][["breaks"]],
                                    labels = variables[[v]][["labels"]])
        g <- g + coord_cartesian(ylim = variables[[v]][["limits"]])
        g <- g + scale_fill_gradient(low='black', high='white',trans="log",
                                     breaks=10 ** seq(0,10))
        g <- g + theme_classic()
        g <- g + theme(panel.background = element_rect(fill = "black",
                                                       colour = "black",
                                                       size = 0.5,
                                                       linetype = "solid"))
        plots <- c(plots,list(g))
    }
    finalG <- arrangeGrob(grobs=plots,nrow=length(variables))
    ggsave(file="runDistributions.png", finalG, width = 16, height=9)
}
