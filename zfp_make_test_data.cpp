#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <zfp.h>
#include <glm/glm.hpp>

int main(int argc, char **argv)
{
    using namespace std::chrono;

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <volume.raw> <compression rate>\n";
        return 1;
    }

    const std::string file = argv[1];
    const float compression_rate = std::stof(argv[2]);
    const std::regex match_filename("(\\w+)_(\\d+)x(\\d+)x(\\d+)_(.+)\\.raw");
    auto matches = std::sregex_iterator(file.begin(), file.end(), match_filename);
    if (matches == std::sregex_iterator() || matches->size() != 6) {
        std::cerr << "Unrecognized raw volume naming scheme, expected a format like: "
                  << "'<name>_<X>x<Y>x<Z>_<data type>.raw' but '" << file << "' did not match"
                  << std::endl;
        return 1;
    }

    glm::uvec3 volume_dims(
        std::stoi((*matches)[2]), std::stoi((*matches)[3]), std::stoi((*matches)[4]));
    const std::string volume_type = (*matches)[5];

    size_t voxel_size = 0;
    if (volume_type == "uint8") {
        voxel_size = 1;
    } else if (volume_type == "uint16") {
        voxel_size = 2;
    } else if (volume_type == "float32") {
        voxel_size = 4;
    } else if (volume_type == "float64") {
        voxel_size = 8;
    } else {
        std::cerr << "Unsupported voxel type: " << volume_type << "!\n";
        return 1;
    }

    // Just read the first 64 voxels for now
    const size_t num_voxels =
        size_t(volume_dims.x) * size_t(volume_dims.y) * size_t(volume_dims.z);
    std::vector<float> volume_data(num_voxels, 0.f);
    {
        std::vector<uint8_t> read_data(num_voxels * voxel_size, 0);
        std::ifstream fin(file.c_str(), std::ios::binary);
        fin.read(reinterpret_cast<char *>(read_data.data()), read_data.size());

        if (volume_type == "uint8") {
            for (size_t i = 0; i < num_voxels; ++i) {
                volume_data[i] = read_data[i];
            }
        } else if (volume_type == "uint16") {
            uint16_t *d = reinterpret_cast<uint16_t *>(read_data.data());
            for (size_t i = 0; i < num_voxels; ++i) {
                volume_data[i] = d[i];
            }
        } else if (volume_type == "float32") {
            float *d = reinterpret_cast<float *>(read_data.data());
            for (size_t i = 0; i < num_voxels; ++i) {
                volume_data[i] = d[i];
            }
        } else if (volume_type == "float64") {
            double *d = reinterpret_cast<double *>(read_data.data());
            for (size_t i = 0; i < num_voxels; ++i) {
                volume_data[i] = d[i];
            }
        }
    }

    zfp_stream *zfp = zfp_stream_open(nullptr);
    float used_compression_rate =
        zfp_stream_set_rate(zfp, compression_rate, zfp_type_float, 3, 0);
    std::cout << "Used compression rate: " << used_compression_rate << "\n";
    if (std::floor(used_compression_rate) != used_compression_rate) {
        std::cout << "Error: non-integer compression rate\n";
        return 1;
    }

    // Just compress the first block. This is also not really the first proper
    // block of 4^3 voxels but just the first 64 voxels in the data, but ok for testing
    zfp_field *field = zfp_field_3d(
        volume_data.data(), zfp_type_float, volume_dims.x, volume_dims.y, volume_dims.z);

    const size_t bufsize = zfp_stream_maximum_size(zfp, field);

    std::vector<uint8_t> compressed_data(bufsize, 0);
    bitstream *stream = stream_open(compressed_data.data(), compressed_data.size());
    zfp_stream_set_bit_stream(zfp, stream);
    zfp_stream_rewind(zfp);

    size_t total_bytes = zfp_compress(zfp, field);
    zfp_field_free(field);

    compressed_data.resize(total_bytes);
    std::cout << "Total compressed size: " << compressed_data.size() << "B\n";

    stream_close(stream);
    zfp_stream_close(zfp);

    // Save out the compressed file
    const std::string out_name =
        file + ".crate" + std::to_string(int(used_compression_rate)) + ".zfp";
    std::ofstream out_file(out_name.c_str(), std::ios::binary);
    out_file.write(reinterpret_cast<const char *>(compressed_data.data()),
                   compressed_data.size());

    return 0;
}
