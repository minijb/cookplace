#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

struct Texture {
    std::string name;
    int width;
    int height;
    std::vector<std::uint32_t> pixels;
};

struct AudioClip {
    std::string name;
    int sample_rate;
    std::vector<float> samples;
};

using Resource = std::variant<Texture, AudioClip>;

std::optional<Resource> load_resource(std::string_view name) {
    if (name == "player") {
        return Resource{Texture{
            "player", 2, 2, std::vector<std::uint32_t>(4, 0xffffffffu)}};
    }
    if (name == "theme") {
        return Resource{AudioClip{
            "theme", 48000, std::vector<float>(256, 0.0F)}};
    }
    return std::nullopt;
}

std::optional<int> find_setting(
    const std::map<std::string, int>& settings,
    std::string_view key) {
    if (const auto it = settings.find(std::string(key)); it != settings.end()) {
        return it->second;
    }
    return std::nullopt;
}

struct DescribeVisitor {
    std::string operator()(const Texture& texture) const {
        return "texture " + texture.name + ": " +
               std::to_string(texture.width) + "x" +
               std::to_string(texture.height) + ", pixels=" +
               std::to_string(texture.pixels.size());
    }

    std::string operator()(const AudioClip& clip) const {
        return "audio " + clip.name + ": sample_rate=" +
               std::to_string(clip.sample_rate) + ", samples=" +
               std::to_string(clip.samples.size());
    }
};

void print_settings(const std::map<std::string, int>& settings) {
    for (const auto& [name, value] : settings) {
        std::cout << "setting " << name << '=' << value << '\n';
    }
}

int main() {
    const std::map<std::string, int> settings{{"width", 1920}, {"height", 1080}};
    print_settings(settings);

    const auto width = find_setting(settings, "width");
    assert(width.has_value() && *width == 1920);

    const auto missing_setting = find_setting(settings, "fullscreen");
    assert(!missing_setting.has_value());

    const std::string requested_name = "player";
    const std::string_view requested_view = requested_name;
    auto resource = load_resource(requested_view);
    assert(resource.has_value());

    std::cout << std::visit(DescribeVisitor{}, *resource) << '\n';

    const auto& texture = std::get<Texture>(*resource);
    const std::string_view texture_name = texture.name;
    assert(texture_name == "player");

    const auto missing_resource = load_resource("missing");
    assert(!missing_resource.has_value());

    {
        Texture temporary{
            "temporary", 4, 4, std::vector<std::uint32_t>(16, 0xff0000ffu)};
        std::cout << "scope owns " << temporary.pixels.size()
                  << " pixels\n";
    }

    std::cout << "scope exited; string/vector storage was released automatically\n";
}
