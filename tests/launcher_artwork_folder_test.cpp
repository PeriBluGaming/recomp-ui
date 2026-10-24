#include "launcher_artwork.h"
#include <cassert>
#include <iostream>
using namespace launcher_artwork;
int main(int argc,char** argv) {
    assert(argc==2);const auto qa=fs::u8path(argv[1]);
    fs::remove_all(qa); // a previous run's tree; the test owns this directory
    fs::create_directories(qa);const auto home=qa/"portable"/"artwork";
    const std::vector<std::string> regions={"SCES-01330","SCES-01331","SLPS-01144"};
    for(const auto& serial:regions) {
        const auto standard=home/serial;fs::create_directories(standard);
        assert(folder(home,serial)==fs::absolute(standard).lexically_normal());
        std::ofstream(standard/"front.png")<<"fixture";
        std::ofstream(standard/"back.JPG")<<"fixture";
        std::ofstream(standard/"disc.tga")<<"fixture";
        std::ofstream(standard/"manual.pdf")<<"%PDF-fixture";
        auto c=scan(home,serial);assert(c.images.size()==3);
        assert(rank(c.images[0])==0 && rank(c.images[1])==1 && rank(c.images[2])==2);
        const auto custom=qa/fs::u8path(u8"Other covers \u00c4 \u65e5\u672c folder")/serial;
        fs::create_directories(custom);
        for(int i=0;i<40;++i)std::ofstream(custom/(std::to_string(i)+".png"))<<"fixture";
        assert(select_folder(home,serial,custom));
        assert(folder(home,serial)==fs::absolute(custom).lexically_normal());
        assert(scan(home,serial).images.size()==40); // no old 32-image truncation
        assert(!select_folder(home,serial,custom/"missing"));
        assert(scan(home,serial).images.size()==40); // invalid choice preserves saved path
        assert(fs::file_size(standard/"front.png")==7);
        assert(fs::file_size(standard/"manual.pdf")==12); // existing documents untouched
    }
    for(const auto& serial:regions)assert(scan(home,serial).images.size()==40);
    const auto empty=qa/"empty";fs::create_directories(empty);
    assert(select_folder(home,regions[0],empty));assert(scan(home,regions[0]).images.empty());
    assert(scan(home,regions[1]).images.size()==40 && scan(home,regions[2]).images.size()==40);
    fs::remove(empty);assert(scan(home,regions[0]).images.empty());
    assert(!select_folder(home,"../bad",qa));assert(scan(home,"../bad").images.empty());
    assert(supported("T-1234G") && supported("MK-81000") && !supported("") &&
           !supported("..") && !supported("a/b") && !supported(std::string(65,'A')));
    // Internal folders are relative, so the preference survives moving the portable tree.
    assert(select_folder(home,regions[0],home/regions[1]));
    const auto moved=qa/"moved portable";fs::rename(home.parent_path(),moved);
    assert(scan(moved/"artwork",regions[0]).images.size()==3);
    // External choices remain absolute, with independent state per region.
    assert(scan(moved/"artwork",regions[1]).images.size()==40);
    std::ofstream(moved/"artwork"/regions[2]/"cover-folder.txt",std::ios::binary)<<std::string(65537,'x');
    assert(scan(moved/"artwork",regions[2]).images.size()==3);
    assert(step(0,3,-1)==2 && step(2,3,1)==0 && step(0,0,1)==0);
    auto fit_size=fit(800,400,200,200);assert(fit_size.w==200 && fit_size.h==100);
    assert(fs::file_size(moved/"artwork"/regions[0]/"front.png")==7);
    std::cout<<"PASS: regional folders, persistence, relocation, Unicode, empty/missing/invalid, 40 images, navigation and aspect ratio; no image writes.\n";
}
