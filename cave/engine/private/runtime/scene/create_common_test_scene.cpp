// @TODO : DELETE THIS FILE!!!
#if 0
Scene* CreateBoxScene() {
    Scene* scene = new Scene;
    CRASH_NOW();
    scene->m_physicsMode = PhysicsMode::SIMULATION;

    auto root = EntityFactory::CreateTransformEntity(*scene, "root");
    scene->m_root = root;

    Vector2i frame_size = DVAR_GET_IVEC2(resolution);
    // main camera
    {
        auto main_camera = EntityFactory::CreatePerspectiveCameraEntity(*scene, "main_camera", frame_size.x, frame_size.y);
        auto camera = scene->GetComponent<CameraComponent>(main_camera);
        DEV_ASSERT(camera);
        camera->SetPosition(Vec3f(0, 4, 15));
        camera->SetPrimary();
        scene->AttachChild(main_camera, root);
    }
    // add a light
    if constexpr (0) {
        auto id = EntityFactory::CreatePointLightEntity(*scene, "point_light", Vec3f(0, 0, 0));
        scene->AttachChild(id, root);
        LightComponent* light = scene->GetComponent<LightComponent>(id);
        light->SetCastShadow();
    }

    auto world = EntityFactory::CreateTransformEntity(*scene, "world");
    scene->AttachChild(world, root);

    int mat_counter = 0;
    auto create_material = [&](const std::string& p_name) {
        unused(p_name);
        auto id = EntityFactory::CreateMaterialEntity(*scene, std::format("{}_{}", "p_name", mat_counter++));
        MaterialComponent* mat = scene->GetComponent<MaterialComponent>(id);
        if (p_name == "white") {
            mat->metallic = 0.3f;
            mat->roughness = 0.7f;
        } else if (p_name == "green") {
            mat->baseColor = Vecf(0, 1, 0, 1);
            mat->metallic = 0.3f;
            mat->roughness = 0.7f;
        } else if (p_name == "red") {
            mat->baseColor = Vecf(1, 0, 0, 1);
            mat->metallic = 0.3f;
            mat->roughness = 0.7f;
        }
        return id;
    };

    {
        constexpr float s = 5.0f;
        struct {
            std::string name;
            Mat4f transform;
            ecs::Entity material;
        } wall_info[] = {
            { "wall_up", Translate(Vec3f(0, s, 0)), create_material("white") },
            { "wall_down", Translate(Vec3f(0, -s, 0)), create_material("white") },
            { "wall_left", Rotate(Degree(+90.0f), Vec3f::UnitZ) * Translate(Vec3f(0, s, 0)), create_material("red") },
            { "wall_right", Rotate(Degree(+90.0f), Vec3f::UnitZ) * Translate(Vec3f(0, -s, 0)), create_material("green") },
            { "wall_back", Rotate(Degree(+90.0f), Vec3f::UnitX) * Translate(Vec3f(0, -s, 0)), create_material("white") },
        };

        for (size_t i = 0; i < std::size(wall_info); ++i) {
            const auto& info = wall_info[i];
            auto wall = EntityFactory::CreateCubeEntity(*scene, info.name, info.material, Vec3f(s, 0.2f, s), info.transform);
            scene->AttachChild(wall, world);
        }
    }

    return scene;
}
#endif
