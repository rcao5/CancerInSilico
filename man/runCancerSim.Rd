% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/CancerModels.R
\name{runCancerSim}
\alias{runCancerSim}
\title{\code{runCancerSim} runs a cell-based model of cancer}
\usage{
runCancerSim(initialNum, runTime, density = 0.01, cycleLengthDist = 12,
  inheritGrowth = FALSE, outputIncrement = 6, randSeed = 0,
  modelType = "DrasdoHohme2003", ...)
}
\arguments{
\item{initialNum}{how many cells initially (integer)}

\item{runTime}{how long the simulation runs (model hours)}

\item{density}{the density the cells are seeded at, must be in (0,0.1]}

\item{cycleLengthDist}{cycle time distribution}

\item{inheritGrowth}{whether or not daughter cells have the same cycle-length as parents}

\item{outputIncrement}{time increment to print status at}

\item{randSeed}{seed for the model}

\item{modelType}{the name of the cell-based model to use}

\item{...}{model specific parameters (depends on modelType)}
}
\value{
A CellModel containing all info from the model run
}
\description{
\code{runCancerSim} runs a cell-based model of cancer
}
\details{
This function provides a centralized R interface to run c++ code for cell-based models implemented in this package. Standard parameters, as well as model-specific parameters, are passed in to this function along with a model name. This function then runs the model and returns a CellModel object containing all the information from the model. This object can then be accessed with various functions designed to interact with the class. To see a list of available functions, there is a show() command implemented for CellModel objects.
}
\examples{
runCancerSim(1,4)
}

