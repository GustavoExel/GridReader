#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/filesystem.hpp>
#include <cgnslib.h>
#include <cgns_io.h>

void readData(std::string basePath, std::string &filePath, int *sizes, std::vector<double> &timeSteps, std::vector<std::vector<double> > &transientFields) {
    std::ifstream data(basePath + "/data.txt");
    data >> filePath;
    data >> *sizes;
    data >> *(sizes+1);
    data >> *(sizes+2);

    std::ifstream fields(basePath + "/fields.txt");
    std::string line;
    std::stringstream ss;
    std::vector<double> field;
    double val;
    while ( getline(fields, line) )
    {
        ss.clear(); ss << line;
        field.clear();
        while (ss >> val) field.emplace_back( val );
        transientFields.emplace_back( field );
        line="";
    }

    std::ifstream steps(basePath + "/steps.txt");
    double timeStep;
    while ( steps >> timeStep ) timeSteps.emplace_back( timeStep );

    data.close();
    fields.close();
    steps.close();
}

int main(int argc, char *argv[]) {
    boost::filesystem::path file, temporaryfile;
    std::string filePath;
    std::string basePath(static_cast<boost::filesystem::path>(argv[0]).parent_path().c_str());

    float fileVersion;
    int fileIndex, baseIndex=1, zoneIndex, solutionIndex=0, fieldIndex;
    int fileType, cellDimension, physicalDimension;
    int sizes[3];

    char buffer[1024];
    std::vector<double> timeValues;
    std::vector< std::vector<double> > transientFields;

    readData(basePath, filePath, &sizes[0], timeValues, transientFields);
    file = filePath;
    temporaryfile = filePath + ".temp";

    // Check if file exists / is valid
    if (cg_is_cgns(file.c_str(), &fileType))
        throw std::runtime_error(std::string() + " - The file is not a valid cgns file");

    // Convert to HDF
    if (fileType == 1) {
        int adfFileIndex, hdfFileIndex;
        cgio_open_file (file.c_str(), CGIO_MODE_READ, CGIO_FILE_ADF, &adfFileIndex);
        cgio_open_file(boost::filesystem::absolute(temporaryfile).c_str(), CGIO_MODE_WRITE, CGIO_FILE_HDF5, &hdfFileIndex);
        cgio_copy_file(adfFileIndex, hdfFileIndex, 1);
        cgio_close_file(adfFileIndex);
        cgio_close_file(hdfFileIndex);
        boost::filesystem::copy_file(temporaryfile, file, boost::filesystem::copy_option::overwrite_if_exists);
        boost::filesystem::remove_all(temporaryfile);
    }

    // Open file
    cg_open(file.c_str(), 2, &fileIndex);
    cg_version(fileIndex, &fileVersion);
    if (fileVersion <= 3.10)
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " - File version (" + std::to_string(fileVersion) + ") is older than 3.11");
    
    cg_base_read(fileIndex, baseIndex, buffer, &cellDimension, &physicalDimension);
    cg_nzones(fileIndex, baseIndex, &zoneIndex);
    cg_zone_read(fileIndex, baseIndex, zoneIndex, buffer, sizes);

    std::string fieldName = "numerical data";
    for ( unsigned counter = 1; counter <= timeValues.size(); ++counter ) {
        std::string solutionName = std::string("TimeStep") + std::to_string(counter);
        cg_sol_write(fileIndex, baseIndex, zoneIndex, solutionName.c_str(), GridLocation_t(2), &solutionIndex);
        
        for (unsigned j = 0; j < counter; ++j) {
            cg_field_write(fileIndex, baseIndex, zoneIndex, solutionIndex, RealDouble, fieldName.c_str(), &transientFields[j][0], &fieldIndex);
        }    
    }

    int numberOfTimeSteps = timeValues.size();
    cg_biter_write(fileIndex, baseIndex, "TimeIterativeValues", timeValues.size());
    cg_goto(fileIndex, baseIndex, "BaseIterativeData_t", 1, nullptr);
    cg_array_write("TimeValues", RealDouble, 1, &numberOfTimeSteps, &timeValues[0]);
    cg_simulation_type_write(fileIndex, baseIndex, TimeAccurate);
}