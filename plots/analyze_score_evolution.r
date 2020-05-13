source("plot_tools.r")

args <- commandArgs(TRUE)

if (length(args) < 1) {
    cat("Usage: ... <path_to_experiments>\n");
    quit(status=1)
}

categories <- autoCategories(args[1], "results.csv")
print(categories)

data <- gatherCategories(categories)
head(data)

g <- ggplot(data, aes(x=elapsed,y=score,color=category,group=path))
g <- g + geom_line(size=0.1)
g <- g + geom_point(size=0.5)
# Not displaying confidence interval because the different points are not
# independent at all.
## g <- g + geom_smooth(aes(group=category), se = FALSE)
ggsave(paste(args[1], "score_evolution.png", sep="/"),width=8,height=6)
