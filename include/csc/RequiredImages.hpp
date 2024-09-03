#ifndef CSC_REQUIREDIMAGES_HPP
#define CSC_REQUIREDIMAGES_HPP

namespace csc {

class ImageManager;

namespace required {

auto manager_with_required_images() -> csc::ImageManager;

}
}  // namespace csc

#endif  // CSC_REQUIREDIMAGES_HPP