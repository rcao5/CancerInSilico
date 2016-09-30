#'\code{runCancerSim} Runs the model
#'
#' @details This function provides a centralized R interface to run c++ code for cell-based models implemented in this package. Standard parameters, as well as model-specific parameters, are passed in to this function along with a model name. This function then runs the model and returns a CellModel object containing all of the information from the model. This object can then be accessed with various functions designed to interact with the class. To see a list of available functions, there is a show() command implemented for CellModel objects.
#' @param initialNum how many cells initially
#' @param runTime how long the simulation runs in real cellular time (hours)
#' @param density the density the cells are seeded at
#' @param cycleLengthDist cycle time distribution
#' @param drugEffect distribution of drug effects
#' @param inheritGrowth whether or not daughter cells have the same cycle-length as parents
#' @param outputIncrement time increment to print status at
#' @param randSeed seed for the model
#' @param modelType the name of the cell-based model to use
#' @param ... model specific parameters (depends on modelType)
#' @return A CellModel containing all info from the model run
#' @examples
#' runCancerSim(1,8)
#' @export

runCancerSim <- function(initialNum,
                         runTime,
                         density = 0.01,
                         cycleLengthDist = 12,
                         drugEffect = getDrugEffect(cycleLengthDist = cycleLengthDist),
                         inheritGrowth = FALSE,
                         outputIncrement = 6,
                         recordIncrement = 0.25,
                         randSeed = 0,
                         modelType = "DrasdoHohme2003",
                         drugTime = 0.0,
                         ...)

{

    if (modelType != "DrasdoHohme2003") {

      stop("invalid model type")

    } else {

      return (runDrasdoHohme(initialNum,
                             runTime,
                             density,
                             cycleLengthDist,
                             inheritGrowth, 
                             outputIncrement,
                             recordIncrement,
                             randSeed,
                             drugEffect,
                             drugTime,
                             ...))

    }

}

#' \code{runDrasdoHohme} Runs the model from the Drasdo, Hohme paper
#'
#' @details  

runDrasdoHohme <- function(initialNum,
                           runTime,
                           density,
                           cycleLengthDist,
                           inheritGrowth,
                           outputIncrement,
                           recordIncrement,
                           randSeed,
                           drugEffect,
                           drugTime,
                           ...)
  
{
  
    nG <- list(...)$nG
    if (is.null(nG)) {nG = 24}

    epsilon <- list(...)$epsilon
    if (is.null(epsilon)) {epsilon = 10}

    if (density > 0.1) {

        message("density too high to seed efficiently\n")
        stop()

    }
  
    delta <- 0.2 ## must be less than 4 or calculations break
  
    #timeIncrement is the time between each timestep
    timeIncrement = delta / (4 * nG * (4 - sqrt(2)))
    max_incr = delta * (min(cycleLengthDist) - 1) / (8 * nG * (sqrt(2) - 1))

    if (timeIncrement > max_incr) {

        timeIncrement = max_incr

    }

    boundary <- 1
    maxDeform <- 2 * timeIncrement * nG * (4 - sqrt(2))
    grRates <- 2 * (sqrt(2) - 1) * timeIncrement * nG / (cycleLengthDist - 1)
    mcSteps <- ceiling(runTime / timeIncrement)
    maxTranslation <- delta / 2
    maxRotate <- acos((16 + delta ^ 2 - 4 * delta) / 16)

    outputIncrement2 <- floor(outputIncrement / timeIncrement)
    recordIncrement2 <- floor(recordIncrement / timeIncrement)  

    if (recordIncrement == 0) {

        recordIncrement2 <- 1

    }
  
    for (i in 1:length(drugEffect)) {

        drugEffect[[i]][1] <- 2 * (sqrt(2) - 1) * timeIncrement * nG / (drugEffect[[i]][1] - 1)

    }
  
    output <- CellModel(initialNum,
                      mcSteps,
                      density,
                      maxTranslation,
                      maxDeform,
                      maxRotate,
                      epsilon, 
                      delta, 
                      outputIncrement2,
                      randSeed,
                      drugEffect, 
                      grRates, 
                      inheritGrowth,
                      nG, 
                      timeIncrement, 
                      recordIncrement2, 
                      drugTime, 
                      boundary)
    
    cellMat <- new("CellModel",
                 mCells = output,
                 mInitialNumCells = initialNum,
                 mRunTime = runTime,
                 mInitialDensity = density,
                 mInheritGrowth = inheritGrowth,
                 mOutputIncrement = outputIncrement,
                 mRandSeed = randSeed,
                 mEpsilon = epsilon,
                 mNG = nG,
                 mTimeIncrement = timeIncrement,
                 mRecordIncrement = recordIncrement,
                 mCycleLengthDist = cycleLengthDist)

    return(cellMat)
  
}