#include "mapper.h"
#include "nrom/nrom.h"

NameTableMirroring Mapper::getNameTableMirroring() {
    return static_cast<NameTableMirroring>(cartridge.getNameTableMirroring());
}

std::shared_ptr<Mapper> Mapper::createMapper(MapperType mapper_t, Cartridge& cart) {
    std::shared_ptr<Mapper> ret(nullptr);
    switch (mapper_t) {
        case NROM:
            ret = std::make_shared<MapperNROM>(cart);
            break;
        default:
            std::cout << "[NEMU] Error: Other Mappers Not Supported.\n";
            break;
    }
    return ret;
}