#include "csc/RequiredImages.hpp"

#include <chrono>

#include "csc/ImageManager.hpp"
#include "csc/ImageRecord.hpp"

using ImageRecord = csc::ImageRecord;
using Genre = csc::ImageRecord::Genre;
using namespace std::literals::chrono_literals;
using namespace csc::date::literals;  // NOLINT

auto csc::required::manager_with_required_images() -> csc::ImageManager {
  return ImageManager{ImageRecord{"Andromeda Galaxy",
                                  "Image of the Andromeda Galaxy",
                                  Genre::Astronomy(),
                                  {2023y / std::chrono::January / 1d, 0_h},
                                  "Images/Andromeda.png"},

                      ImageRecord{"Lanyon QUB",
                                  "An Image of the QUB Lanyon building.",
                                  Genre::Architecture(),
                                  {2023y / std::chrono::January / 2d, 0_h},
                                  "Images/LanyonQUB.png"},

                      ImageRecord{"Kermit Plays Golf",
                                  "An image of Kermit the frog playing golf.",
                                  Genre::Sport(),
                                  {2023y / std::chrono::January / 3d, 0_h},
                                  "Images/KermitGolf.png"},

                      ImageRecord{"Mourne Mountains",
                                  "A panoramic view of the Mourne mountains.",
                                  Genre::Landscape(),
                                  {2023y / std::chrono::January / 4d, 0_h},
                                  "Images/Mournes.png"},

                      ImageRecord{"Homer Simpson",
                                  "Homer Simpson- A portrait of the man.",
                                  Genre::Portrait(),
                                  {2023y / std::chrono::January / 3d, 0_h},
                                  "Images/Homer.png"},

                      ImageRecord{"Red Kite",
                                  "A Red Kite bird of prey in flight.",
                                  Genre::Nature(),
                                  {2023y / std::chrono::January / 4d, 0_h},
                                  "Images/RedKite.png"},

                      ImageRecord{"Central Park",
                                  "An overhead view of Central Park  York USA.",
                                  Genre::Aerial(),
                                  {2023y / std::chrono::January / 5d, 0_h},
                                  "Images/CentralPark.png"},

                      ImageRecord{"Apples",
                                  "A bunch of apples.",
                                  Genre::Food(),
                                  {2023y / std::chrono::January / 6d, 0_h},
                                  "Images/Apples.png"},

                      ImageRecord{"Programming Meme",
                                  "A Chat GPT programming meme.",
                                  Genre::Other(),
                                  {2023y / std::chrono::January / 7d, 0_h},
                                  "Images/ChatGPT.png"}};
}