class_name ClusterWrangler extends Node

var bullets: Array[BulletLinear] = []

func add_bullet(bullet: BulletLinear) -> void:
    bullets.append(bullet)
    bullet.destroyed.connect(_on_bullet_destroyed.bind(len(bullets) - 1))

func _on_bullet_destroyed(idx: int) -> void:
    bullets.remove_at(idx)
    update_bullets()

# call after adding all bullets
func update_bullets() -> void:
    if len(bullets) == 1:
        bullets[0].angular_offset = 0
        bullets[0].angular_radius = 0
    else:
        for idx in len(bullets):
            var bullet := bullets[idx]
            bullet.angular_offset = TAU * float(idx) / float(len(bullets))
