class_name ClusterWrangler extends Node

var bullets: Array[BulletLinear] = []

func add_bullet(bullet: BulletLinear) -> void:
    bullets.append(bullet)
    bullet.destroyed.connect(_on_bullet_destroyed.bind(bullet))

func _on_bullet_destroyed(bullet: BulletLinear) -> void:
    bullets.remove_at(bullets.find(bullet))
    update_bullets()

# call after adding all bullets
func update_bullets() -> void:
    if len(bullets) == 1:
        if bullets[0]:
            bullets[0].angular_offset = 0
            bullets[0].angular_radius = 0
        return
    
    for idx in len(bullets):
        var bullet := bullets[idx]
        if bullet:
            bullet.angular_offset = TAU * float(idx) / float(len(bullets))
